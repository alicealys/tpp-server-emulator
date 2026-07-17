#include <std_include.hpp>

#include "event_rankings.hpp"
#include "player_records.hpp"
#include "players.hpp"

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
				for (auto i = 0; i < event_count; i++)
				{
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
			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(event_ranking::table)
						.set(event_ranking::table.value = event_ranking::table.value + count)
							.where(event_ranking::table.player_id == player_id && event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
				);

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool set_value_if_bigger(const std::uint64_t player_id, const event_type event_id, const std::int32_t value)
		{
			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(event_ranking::table)
						.set(event_ranking::table.value = value)
							.where(event_ranking::table.player_id == player_id && 
								   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && event_ranking::table.value < value)
				);

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
						.set(event_ranking::table.value = 0, event_ranking::table.player_rank = 0)
							.where(event_ranking::table.event_id != static_cast<std::uint32_t>(ep_earned) &&
								   event_ranking::table.event_id != static_cast<std::uint32_t>(league_point_total))
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
						.set(event_ranking::table.value = 0, event_ranking::table.player_rank = 0)
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
		db.run_query("mgstpp.event_rankings.update_entries_league", static_cast<std::uint32_t>(league_point_total));
	}

	std::chrono::seconds get_last_update()
	{
		return std::chrono::duration_cast<std::chrono::seconds>(last_update.time_since_epoch());
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.event_rankings.create");
		}

		void run_tasks(database_t& database) override
		{
			update_entries(database);
		}
	};
}

REGISTER_TABLE(database::event_rankings::table, -1)
