#include <std_include.hpp>

#include "event_rankings.hpp"
#include "player_records.hpp"
#include "players.hpp"
#include "fob_events.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::event_rankings
{
	namespace
	{
		std::unordered_map<std::string, event_ranking_lookup_type> lookup_type_map =
		{
			{"BEST", lookup_best},
			{"AROUND", lookup_around},
			{"GRADE", lookup_grade},
		};

		std::unordered_map<std::string, event_class> event_class_map =
		{
			{"SNEAK", class_fob},
			{"SNEAK_EVENT", class_fob_event},
			{"LEAGUE", class_league},
			{"LEAGUE_EVENT", class_league_event},
		};

		std::vector<nameplate_info_t> load_nameplate_info_list()
		{
			std::vector<nameplate_info_t> list;

			auto data = utils::resources::load_json(RESOURCE_NAMEPLATES);
			for (auto i = 0ull; i < data.size(); i++)
			{
				nameplate_info_t entry{};
				entry.id = data[i]["id"].get<std::uint32_t>();
				entry.target.type = data[i]["target"]["type"].get<std::uint8_t>();
				entry.target.event = data[i]["target"]["event"].get<std::uint8_t>();
				entry.target.min = data[i]["target"]["min"].get<std::uint32_t>();
				entry.target.max = data[i]["target"]["max"].get<std::uint32_t>();
				list.emplace_back(entry);
			}

			return list;
		}

		bool is_periodic_event(const std::uint32_t event_id)
		{
			switch (event_id)
			{
				case most_ep_in_mission:
				case cores_reached_defender:
				case cores_reached_no_defender:
				case damage_defender:
				case materials_stolen:
				case total_stealth:
				case unknown:
				case defense_deployments:
				case defense_deployments_support:
				case defense_deployments_support_received:
				case defender_neutralized_intruder:
				case successful_defenses:
				case times_neutralized_by_intruder:
				case fob_event_ranking:
				case league_offsensive_boost:
				case league_defensive_boost:
				case league_grade_boost:
				case league_security_boost:
				case league_vehicles:
				case league_fight_vehicles:
				case league_walker_gears:
				case league_emplacements:
				case league_wins:
				case league_pf_points_exchange:
					return true;
			}

			return false;
		}

		std::uint32_t get_event_total(const std::uint32_t event_id)
		{
			switch (event_id)
			{
				case cores_reached_defender:
					return cores_reached_defender_total;
				case cores_reached_no_defender:
					return cores_reached_no_defender_total;
				case damage_defender:
					return damage_defender_total;
				case materials_stolen:
					return materials_stolen_total;
				case total_stealth:
					return total_stealth_total;
				case defense_deployments:
					return defense_deployments_total;
				case defense_deployments_support:
					return defense_deployments_support_total;
				case defense_deployments_support_received:
					return defense_deployments_support_received_total;
				case defender_neutralized_intruder:
					return defender_neutralized_intruder_total;
				case successful_defenses:
					return successful_defenses_total;
				case times_neutralized_by_intruder:
					return times_neutralized_by_intruder_total;
			}

			return event_invalid;
		}
	}

	std::optional<event_type> get_event_type_from_id(const std::uint32_t id, const std::string& type)
	{
		const auto upper = utils::string::to_upper(type);
		const auto iter = event_class_map.find(upper);
		if (iter == event_class_map.end())
		{
			return {};
		}

		if ((iter->second == class_fob || iter->second == class_fob_event) && id <= fob_event_end)
		{
			return static_cast<event_type>(id);
		}
		else if ((iter->second == class_league || iter->second == class_league_event) && id <= (league_event_end - fob_event_end))
		{
			return static_cast<event_type>(id + league_event_start);
		}

		return {};
	}

	std::optional<event_ranking_lookup_type> get_lookup_type_from_name(const std::string& name)
	{
		const auto upper = utils::string::to_upper(name);
		const auto iter = lookup_type_map.find(upper);
		if (iter == lookup_type_map.end())
		{
			return {};
		}

		return {iter->second};
	}

	const std::vector<nameplate_info_t>& get_nameplate_info_list()
	{
		static const auto list = load_nameplate_info_list();
		return list;
	}

	nameplate_entitlement_list::nameplate_entitlement_list()
	{
		this->resize(0x100);
	}

	bool nameplate_entitlement_list::has_nameplate(const std::uint32_t id) const
	{
		if (id >= this->size())
		{
			return false;
		}

		return this->operator[](id);
	}

	GET_FIELD_C(event_ranking, std::uint64_t, id);
	GET_FIELD_C(event_ranking, std::uint64_t, player_id);
	GET_FIELD_C(event_ranking, std::uint32_t, event_id);
	GET_FIELD_C(event_ranking, std::uint64_t, rank);
	GET_FIELD_C(event_ranking, std::uint64_t, rank_number);
	GET_FIELD_C(event_ranking, std::int32_t, value);
	GET_FIELD_C(event_ranking, std::uint32_t, fob_grade);
	GET_FIELD_C(event_ranking, std::uint32_t, league_grade);
	GET_FIELD_C(event_ranking, std::uint64_t, account_id);

	namespace impl
	{
		template <database_type_t Type>
		void create_entries(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				std::array<bool, event_count> valid{};

				auto list = db.get_database<Type>()->operator()(
					sqlpp::select(event_ranking::table.event_id)
						.from(event_ranking::table)
							.where(event_ranking::table.player_id == player_id));

				for (auto& row : list)
				{
					if (row.event_id < valid.size())
					{
						valid[row.event_id] = true;
					}
				}

				for (auto i = 0; i < event_count; i++)
				{
					if (valid[i])
					{
						continue;
					}

					db.get_database<Type>()->operator()(
						sqlpp::insert_into(event_ranking::table)
							.set(event_ranking::table.player_id = player_id,
								 event_ranking::table.event_id = i)
					);
				}
			});
		}
	
		template <database_type_t Type>
		bool set_event_value(const std::uint64_t player_id, const event_type event_id, const std::int32_t value)
		{
			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(event_ranking::table)
						.set(event_ranking::table.value = value)
							.where(event_ranking::table.player_id == player_id && event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
				);

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool increment_event_value(const std::uint64_t player_id, const event_type event_id, const std::int32_t count)
		{
			const auto total_event = get_event_total(event_id);

			return database::access<bool>([&](database_t& db)
			{
				auto update = sqlpp::update(event_ranking::table)
					.set(event_ranking::table.value = event_ranking::table.value + count);

				if (total_event == event_invalid)
				{
					const auto query = update.where(event_ranking::table.player_id == player_id && 
						event_ranking::table.event_id == static_cast<std::uint32_t>(event_id));

					const auto result = db.get_database<Type>()->operator()(query);
					return result != 0;
				}
				else
				{
					const auto query = update.where(event_ranking::table.player_id == player_id && 
						(event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) ||
						event_ranking::table.event_id == static_cast<std::uint32_t>(total_event)));

					const auto result = db.get_database<Type>()->operator()(query);
					return result != 0;
				}

			});
		}

		template <database_type_t Type>
		bool set_value_if_bigger(const std::uint64_t player_id, const event_type event_id, const std::int32_t value)
		{
			const auto total_event = get_event_total(event_id);

			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(event_ranking::table)
						.set(event_ranking::table.value = value)
							.where(event_ranking::table.player_id == player_id && 
								   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && event_ranking::table.value < value)
				);

				if (total_event != event_invalid)
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(event_ranking::table)
							.set(event_ranking::table.value = value)
								.where(event_ranking::table.player_id == player_id && 
									   event_ranking::table.event_id == static_cast<std::uint32_t>(total_event) && event_ranking::table.value < value)
					);
				}

				return result != 0;
			});
		}

		template <database_type_t Type>
		void reset_periodic_values()
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(event_ranking::table)
						.set(event_ranking::table.value = 0, event_ranking::table.player_rank = 0, event_ranking::table.player_rank_number = 0)
							.where(event_ranking::table.event_id != static_cast<std::uint32_t>(ep_earned) &&
								   event_ranking::table.event_id != static_cast<std::uint32_t>(league_point_total) &&
								   event_ranking::table.event_id < static_cast<std::uint32_t>(event_total_start))
					);
			});
		}
		
		template <database_type_t Type>
		void reset_values(const event_type event_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(event_ranking::table)
						.set(event_ranking::table.value = 0, event_ranking::table.player_rank = 0, event_ranking::table.player_rank_number = 0)
							.where(event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
					);
			});
		}

		template <database_type_t Type>
		std::optional<std::uint64_t> get_player_rank(const std::uint64_t player_id, const event_type event_id)
		{
			return database::access<std::optional<std::uint64_t>>([&](database_t& db)
				-> std::optional<std::uint64_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(event_ranking::table.player_rank)
						.from(event_ranking::table)
							.where(event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
								   event_ranking::table.player_id == player_id)
					);

				if (results.empty())
				{
					return {};
				}

				return {results.front().player_rank};
			});
		}
		
		template <database_type_t Type>
		std::int32_t get_entry_value(const std::uint64_t player_id, const event_type event_id)
		{
			return database::access<std::int32_t>([&](database_t& db)
				-> std::int32_t
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(event_ranking::table.value)
						.from(event_ranking::table)
							.where(event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
								   event_ranking::table.player_id == player_id)
					);

				if (results.empty())
				{
					return 0;
				}

				return static_cast<std::int32_t>(results.front().value);
			});
		}

		template <database_type_t Type>
		std::optional<event_ranking> get_player_entry(const std::uint64_t player_id, const event_type event_id)
		{
			return database::access<std::optional<event_ranking>>([&](database_t& db)
				-> std::optional<event_ranking>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(event_ranking::table))
						.from(event_ranking::table)
							.where(event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
								   event_ranking::table.player_id == player_id)
					);

				if (results.empty())
				{
					return {};
				}

				return event_ranking(results.front());
			});
		}
				
		template <database_type_t Type>
		std::optional<std::uint64_t> get_grade_offset(const std::uint32_t grade, const event_type event_id, const bool league)
		{
			return database::access<std::optional<std::uint64_t>>([&](database_t& db)
				-> std::optional<std::uint64_t>
			{
				auto joined_tables = event_ranking::table
						.join(player_records::player_record::table)
							.on(event_ranking::table.player_id == player_records::player_record::table.player_id)
						.join(players::player::table)
							.on(event_ranking::table.player_id == player_records::player_record::table.player_id && event_ranking::table.player_id == players::player::table.id);

				if (league)
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::select(sqlpp::all_of(event_ranking::table), player_records::player_record::table.fob_grade, player_records::player_record::table.league_grade,
									  players::player::table.account_id)
							.from(joined_tables)
								.where(event_ranking::table.player_rank != 0 && 
									   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
									   player_records::player_record::table.league_grade <= grade)
									.order_by(event_ranking::table.player_rank.asc())
										.limit(1u)
						);


					if (results.empty())
					{
						return {};
					}

					return results.front().player_rank_number;
				}
				else
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::select(sqlpp::all_of(event_ranking::table), player_records::player_record::table.fob_grade, player_records::player_record::table.league_grade,
									  players::player::table.account_id)
							.from(joined_tables)
								.where(event_ranking::table.player_rank != 0 && 
									   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
									   player_records::player_record::table.fob_grade <= grade)
									.order_by(event_ranking::table.player_rank.asc())
										.limit(1u)
						);


					if (results.empty())
					{
						return {};
					}

					return results.front().player_rank_number;
				}
			});
		}
						
		template <database_type_t Type>
		std::uint32_t snap_grade(const std::uint32_t grade, const event_type event_id, const bool league)
		{
			return database::access<uint32_t>([&](database_t& db)
				-> std::uint32_t
			{
				auto joined_tables = event_ranking::table
						.join(player_records::player_record::table)
							.on(event_ranking::table.player_id == player_records::player_record::table.player_id)
						.join(players::player::table)
							.on(event_ranking::table.player_id == player_records::player_record::table.player_id && event_ranking::table.player_id == players::player::table.id);

				if (league)
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::select(sqlpp::all_of(event_ranking::table), player_records::player_record::table.fob_grade, player_records::player_record::table.league_grade,
									  players::player::table.account_id)
							.from(joined_tables)
								.where(event_ranking::table.player_rank != 0 && 
									   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
									   player_records::player_record::table.league_grade >= grade)
									.order_by(player_records::player_record::table.league_grade.asc())
										.limit(1u)
						);


					if (results.empty())
					{
						grade;
					}

					return static_cast<std::uint32_t>(results.front().league_grade);
				}
				else
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::select(sqlpp::all_of(event_ranking::table), player_records::player_record::table.fob_grade, player_records::player_record::table.league_grade,
									  players::player::table.account_id)
							.from(joined_tables)
								.where(event_ranking::table.player_rank != 0 && 
									   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && 
									   player_records::player_record::table.fob_grade >= grade)
									.order_by(player_records::player_record::table.fob_grade.asc())
										.limit(1u)
						);


					if (results.empty())
					{
						return grade;
					}

					return static_cast<std::uint32_t>(results.front().fob_grade);
				}
			});
		}

		template <database_type_t Type>
		std::vector<event_ranking> get_entries(const event_type event_id, const std::uint64_t offset, const std::uint32_t num)
		{
			return database::access<std::vector<event_ranking>>([&](database_t& db)
				-> std::vector<event_ranking>
			{
				std::vector<event_ranking> list;

				auto joined_tables = event_ranking::table
					.join(player_records::player_record::table)
						.on(event_ranking::table.player_id == player_records::player_record::table.player_id)
					.join(players::player::table)
						.on(event_ranking::table.player_id == player_records::player_record::table.player_id && event_ranking::table.player_id == players::player::table.id);

				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(event_ranking::table), player_records::player_record::table.fob_grade, player_records::player_record::table.league_grade,
								  players::player::table.account_id)
						.from(joined_tables)
							.where(event_ranking::table.player_rank != 0 && event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
								.order_by(event_ranking::table.player_rank_number.asc())
									.limit(num)
										.offset(offset)
					);

				for (auto& row : results)
				{
					event_ranking entry{row};
					entry.set_fob_grade(static_cast<std::uint32_t>(row.fob_grade));
					entry.set_league_grade(static_cast<std::uint32_t>(row.league_grade));
					entry.set_account_id(row.account_id);
					list.emplace_back(entry);
				}

				return list;
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(event_ranking::table)
						.where(event_ranking::table.player_id == player_id));
			});
		}
	}

	void create_entries(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::create_entries, player_id);
	}

	bool set_event_value(const std::uint64_t player_id, const event_type event_id, const std::int32_t value)
	{
		RUN_IMPL(impl::set_event_value, player_id, event_id, value);
	}

	bool increment_event_value(const std::uint64_t player_id, const event_type event_id, const std::int32_t count)
	{
		RUN_IMPL(impl::increment_event_value, player_id, event_id, count);
	}

	bool set_value_if_bigger(const std::uint64_t player_id, const event_type event_id, const std::int32_t value)
	{
		RUN_IMPL(impl::set_value_if_bigger, player_id, event_id, value);
	}

	void reset_periodic_values()
	{
		RUN_IMPL(impl::reset_periodic_values);
	}

	void reset_values(const event_type type)
	{
		RUN_IMPL(impl::reset_values, type);
	}

	std::optional<std::uint64_t> get_player_rank(const std::uint64_t player_id, const event_type event_id)
	{
		RUN_IMPL(impl::get_player_rank, player_id, event_id);
	}

	std::int32_t get_entry_value(const std::uint64_t player_id, const event_type event_id)
	{
		RUN_IMPL(impl::get_entry_value, player_id, event_id);
	}

	std::optional<event_ranking> get_player_entry(const std::uint64_t player_id, const event_type event_id)
	{
		RUN_IMPL(impl::get_player_entry, player_id, event_id);
	}

	std::optional<std::uint64_t> get_grade_offset(const std::uint32_t grade, const event_type event_id, const bool league)
	{
		RUN_IMPL(impl::get_grade_offset, grade, event_id, league);
	}

	std::uint32_t snap_grade(const std::uint32_t grade, const event_type event_id, const bool league)
	{
		RUN_IMPL(impl::snap_grade, grade, event_id, league);
	}

	std::vector<event_ranking> get_entries(const event_type event_id, const std::uint64_t offset, const std::uint32_t num)
	{
		RUN_IMPL(impl::get_entries, event_id, offset, num);
	}

	std::chrono::system_clock::time_point last_update{};

	void update_entries(database_t& db)
	{
		const auto now = std::chrono::system_clock::now();
		if (now - last_update < 10min)
		{
			return;
		}

		last_update = now;

		db.run_query("mgstpp.event_rankings.update_entries", static_cast<std::uint32_t>(league_point_total));
		player_records::update_fob_ranking(db);
	}

	std::chrono::seconds get_last_update()
	{
		return std::chrono::duration_cast<std::chrono::seconds>(last_update.time_since_epoch());
	}

	void update_league(database_t& db)
	{
		db.run_query("mgstpp.event_rankings.update_entries_league", static_cast<std::uint32_t>(league_point_total));
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
	}

	nameplate_entitlement_list get_nameplate_entitlement_list(const std::uint64_t player_id)
	{
		nameplate_entitlement_list list;

		std::array<std::optional<event_ranking>, event_count> rankings;
		std::array<bool, event_count> loaded_ranking{};

		const auto is_event_intermission = database::fob_events::is_intermission();
		const auto& nameplates = get_nameplate_info_list();
		for (auto& info : nameplates)
		{
			if (info.target.event >= event_count || info.id > list.size())
			{
				continue;
			}

			if (is_periodic_event(info.target.event) && !is_event_intermission)
			{
				continue;
			}

			auto& ranking = rankings[info.target.event];
			if (!ranking.has_value() && !loaded_ranking[info.target.event])
			{
				ranking = get_player_entry(player_id, database::event_rankings::event_type(info.target.event));
				loaded_ranking[info.target.event] = true;
			}

			if (!ranking.has_value())
			{
				continue;
			}

			switch (info.target.type)
			{
			case nameplate_target_value:
			{
				if (info.target.max != -1)
				{
					list[info.id] = ranking->get_value() >= info.target.min && ranking->get_value() <= info.target.max;
				}
				else
				{
					list[info.id] = ranking->get_value() >= info.target.min;
				}

				break;
			}
			case nameplate_target_rank:
			{
				if (ranking->get_value() <= 0)
				{
					continue;
				}

				if (info.target.max != -1)
				{
					list[info.id] = ranking->get_rank() >= static_cast<std::uint64_t>(info.target.min) &&
						ranking->get_rank() <= static_cast<std::uint64_t>(info.target.max);
				}
				else
				{
					list[info.id] = ranking->get_rank() >= static_cast<std::uint64_t>(info.target.min);
				}

				break;
			}
			case nameplate_target_rank_number:
			{
				if (ranking->get_value() <= 0)
				{
					continue;
				}

				if (info.target.max != -1)
				{
					list[info.id] = ranking->get_rank_number() >= static_cast<std::uint64_t>(info.target.min) &&
						ranking->get_rank_number() <= static_cast<std::uint64_t>(info.target.max);
				}
				else
				{
					list[info.id] = ranking->get_rank_number() >= static_cast<std::uint64_t>(info.target.min);
				}

				break;
			}
			}
		}

		return list;
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			get_nameplate_info_list();

			database.run_query("mgstpp.event_rankings.create");
		}

		void run_tasks(database_t& database) override
		{
			update_entries(database);
		}
	};
}

REGISTER_TABLE(database::event_rankings::table, -1)
