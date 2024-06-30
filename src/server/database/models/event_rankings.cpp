#include <std_include.hpp>

#include "event_rankings.hpp"
#include "player_records.hpp"
#include "players.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `event_rankings`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	event_id	        int unsigned	not null,
	player_rank		    bigint unsigned	not null default 0,
	value		        int unsigned	not null default 0,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique key `unique_player_event_rankings_type` (`player_id`, `event_id`)
))"

namespace database::event_rankings
{
	namespace
	{
		std::unordered_map<std::string, event_ranking_lookup_type> lookup_type_map =
		{
			{"BEST", lookup_best},
			{"AROUND", lookup_around},
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
	GET_FIELD_C(event_ranking, std::uint32_t, value);
	GET_FIELD_C(event_ranking, std::uint32_t, fob_grade);
	GET_FIELD_C(event_ranking, std::uint32_t, league_grade);
	GET_FIELD_C(event_ranking, std::uint64_t, account_id);

	void create_entries(const std::uint64_t player_id)
	{
		database::access([&](database_t& db)
		{
			for (auto i = 0; i < event_count; i++)
			{
				db->operator()(
					sqlpp::insert_into(event_ranking::table)
						.set(event_ranking::table.player_id = player_id,
							 event_ranking::table.event_id = i)
				);
			}
		});
	}
	
	bool set_event_value(const std::uint64_t player_id, const event_type event_id, const std::uint32_t value)
	{
		return database::access<bool>([&](database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::update(event_ranking::table)
					.set(event_ranking::table.value = value)
						.where(event_ranking::table.player_id == player_id && event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
			);

			return result != 0;
		});
	}

	bool increment_event_value(const std::uint64_t player_id, const event_type event_id, const std::uint32_t count)
	{
		return database::access<bool>([&](database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::update(event_ranking::table)
					.set(event_ranking::table.value = event_ranking::table.value + count)
						.where(event_ranking::table.player_id == player_id && event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
			);

			return result != 0;
		});
	}

	bool set_value_if_bigger(const std::uint64_t player_id, const event_type event_id, const std::uint32_t value)
	{
		return database::access<bool>([&](database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::update(event_ranking::table)
					.set(event_ranking::table.value = value)
						.where(event_ranking::table.player_id == player_id && 
							   event_ranking::table.event_id == static_cast<std::uint32_t>(event_id) && event_ranking::table.value < value)
			);

			return result != 0;
		});
	}

	bool reset_periodic_values()
	{
		return database::access<bool>([&](database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::update(event_ranking::table)
					.set(event_ranking::table.value = 0, event_ranking::table.player_rank = 0)
						.where(event_ranking::table.event_id != static_cast<std::uint32_t>(ep_earned))
				);

			return result != 0;
		});
	}

	std::optional<std::uint64_t> get_player_rank(const std::uint64_t player_id, const event_type event_id)
	{
		return database::access<std::optional<std::uint64_t>>([&](database_t& db)
			-> std::optional<std::uint64_t>
		{
			auto results = db->operator()(
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

			auto results = db->operator()(
				sqlpp::select(sqlpp::all_of(event_ranking::table), player_records::player_record::table.fob_grade, player_records::player_record::table.league_grade,
							  players::player::table.account_id)
					.from(joined_tables)
						.where(event_ranking::table.player_rank != 0 && event_ranking::table.event_id == static_cast<std::uint32_t>(event_id))
							.order_by(event_ranking::table.player_rank.asc())
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

	std::chrono::system_clock::time_point last_update{};

	void update_entries(database_t& db)
	{
		const auto now = std::chrono::system_clock::now();
		if (now - last_update < 10min)
		{
			return;
		}

		last_update = now;

		db->execute(R"(
			update event_rankings event_ranking
			join (
				select
					player_id,
					event_id,
					value,
					case 
						when value = 0 then 0 
						else rank() over (partition by event_id order by value desc)
					end as new_rank
				from event_rankings
			) ranked
			on event_ranking.player_id = ranked.player_id AND event_ranking.event_id = ranked.event_id
			set event_ranking.player_rank = ranked.new_rank;
		)");
	}

	std::chrono::seconds get_last_update()
	{
		auto a = last_update.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::seconds>(last_update.time_since_epoch());
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}

		void run_tasks(database_t& database)
		{
			update_entries(database);
		}
	};
}

REGISTER_TABLE(database::event_rankings::table, -1)
