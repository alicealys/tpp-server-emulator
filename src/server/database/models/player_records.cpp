#include <std_include.hpp>

#include "player_records.hpp"
#include "player_data.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#define TABLE_DEF R"(
create table if not exists `player_records`
(
	id							bigint unsigned	not null	auto_increment,
	player_id					bigint unsigned	not null,
	fob_grade					int not null	default 0,
	prev_fob_grade				int not null	default 0,
	fob_point					int not null	default 0,
	fob_rank					int not null	default 0,
	prev_fob_rank				int not null	default 0,
	is_insurance				boolean			default false,
	league_grade				int not null	default 0,
	prev_league_grade			int not null	default 0,
	league_rank					int not null	default 0,
	prev_league_rank			int not null	default 0,
	league_point				int not null	default 0,
	playtime					int not null	default 0,
	point						int not null	default 0,
	fob_defense_win				int not null	default 0,
	fob_defense_lose			int not null	default 0,
	fob_sneak_win				int not null	default 0,
	fob_sneak_lose				int not null	default 0,
	fob_deploy_emergency_count	int not null	default 0,
	shield_date					datetime not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
))"

namespace database::player_records
{
	std::optional<player_record> find(const std::uint64_t player_id)
	{
		return database::access<std::optional<player_record>>([&](database::database_t& db)
			-> std::optional<player_record>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(player_record::table))
						.from(player_record::table)
							.where(player_record::table.player_id == player_id));

			if (results.empty())
			{
				return {};
			}

			const auto& row = results.front();
			return {player_record(row)};
		});
	}

	player_record find_or_create(const std::uint64_t player_id)
	{
		{
			const auto found = find(player_id);
			if (found.has_value())
			{
				return found.value();
			}
		}

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::insert_into(player_record::table)
					.set(player_record::table.player_id = player_id,
						 player_record::table.shield_date = std::chrono::system_clock::now()));
		});

		const auto found = find(player_id);
		if (!found.has_value())
		{
			throw std::runtime_error("[database::player_records::insert] Insertion failed");
		}

		return found.value();
	}

	void add_sneak_result(const std::uint64_t player_id, const std::uint64_t owner_id, 
		const std::int32_t point_add, const bool is_win, const bool is_sneak)
	{
		database::access([&](database::database_t& db)
		{
			const auto record = find(player_id);
			if (!record.has_value())
			{
				return;
			}

			const auto points = std::max(0, record->get_fob_point() + point_add);

			db->operator()(
				sqlpp::update(player_record::table)
					.set(player_record::table.fob_point = points)
							.where(player_record::table.player_id == player_id)
				);

			if (is_sneak)
			{
				db->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.fob_defense_win = player_record::table.fob_defense_win + static_cast<std::int32_t>(!is_win),
							 player_record::table.fob_defense_lose = player_record::table.fob_defense_lose + static_cast<std::int32_t>(is_win))
								.where(player_record::table.player_id == owner_id)
					);

				db->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.fob_sneak_win = player_record::table.fob_sneak_win + static_cast<std::int32_t>(is_win),
							 player_record::table.fob_sneak_lose = player_record::table.fob_sneak_lose + static_cast<std::int32_t>(!is_win))
								.where(player_record::table.player_id == player_id)
					);
			}
		});
	}

	void sync_prev_values(const std::uint64_t player_id)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_record::table)
					.set(player_record::table.prev_fob_grade = player_record::table.fob_grade,
						 player_record::table.prev_fob_rank = player_record::table.fob_rank)
							.where(player_record::table.player_id == player_id)
				);
		});
	}

	void update_fob_ranking(database_t& db)
	{
		static std::chrono::steady_clock::time_point last_update{};
		const auto now = std::chrono::high_resolution_clock::now();
		if (now - last_update < 10min)
		{
			return;
		}

		last_update = now;

		db->execute(R"(
			with ranked_players as (
				select player_id, fob_point, 
					   (select count(distinct fob_point) + 1 
						from player_records pr2 
						where pr2.fob_point > pr1.fob_point) as new_rank
				from player_records pr1
			)
			update player_records record
			join ranked_players ranked_player on record.player_id = ranked_player.player_id
			set record.fob_rank = ranked_player.new_rank where record.fob_point > 0;
		)");

		static std::vector<std::pair<std::uint32_t, std::uint32_t>> rank_ranges =
		{
			{0, 0},				// 0
			{1, 2},				// 1
			{2, 4},				// 2
			{5, 10},			// 3
			{10, 50},			// 4
			{50, 100},			// 5
			{100, 500},			// 6
			{500, 2000},		// 7
			{2000, 10000},		// 8
			{10000, 100000},	// 9
			{100000, 1000000},	// 10
			{1000000, 1000000},	// 11
		};

		for (auto i = 0; i < rank_ranges.size(); i++)
		{
			const auto& range = rank_ranges[i];
			if (i == rank_ranges.size() - 1)
			{
				db->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.fob_grade = i)
							.where(player_record::table.fob_point >= range.first)
					);
			}
			else
			{
				db->operator()(
					sqlpp::update(player_record::table)
						.set(player_record::table.fob_grade = i)
							.where(player_record::table.fob_point >= range.first && player_record::table.fob_point < range.second)
					);
			}
		}
	}

	std::vector<player_record> find_players_of_grade(const std::uint64_t player_id, const std::uint32_t grade, const std::uint32_t limit)
	{
		return database::access<std::vector<player_record>>([&](database::database_t& db)
			-> std::vector<player_record>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(player_record::table))
						.from(player_record::table)
							.where(player_record::table.fob_grade == grade)
								.order_by(sqlpp::verbatim("rand()").asc())
									.limit(limit));

			std::vector<player_record> list;
			for (auto& row : results)
			{
				list.emplace_back(row);
			}

			return list;
		});
	}

	std::vector<player_record> find_same_grade_players(const std::uint64_t player_id, const std::uint32_t limit)
	{
		return database::access<std::vector<player_record>>([&](database::database_t& db)
			-> std::vector<player_record>
		{
			const auto record = find(player_id);
			if (!record.has_value())
			{
				return {};
			}

			return find_players_of_grade(player_id, record->get_fob_grade(), limit);
		});
	}

	std::vector<player_record> find_higher_grade_players(const std::uint64_t player_id, const std::uint32_t limit)
	{
		return database::access<std::vector<player_record>>([&](database::database_t& db)
			-> std::vector<player_record>
		{
			const auto record = find(player_id);
			if (!record.has_value())
			{
				return {};
			}

			const auto grade = record->get_fob_grade();
			const auto higher_grade = grade == highest_grade ? grade : grade + 1;

			return find_players_of_grade(player_id, higher_grade, limit);
		});
	}

	void set_shield_date(const std::uint64_t player_id, const bool is_win)
	{
		auto date = std::chrono::system_clock::now();
		const auto nuke_count = player_data::get_player_nuke_count(player_id);
		const auto frac = static_cast<float>(nuke_count) / 16.f;
		const auto extra_days = static_cast<std::uint32_t>(frac * 7);

		if (nuke_count != 0)
		{
			date += 24h * extra_days;
		}
		else
		{
			if (!is_win)
			{
				date += 6h;
			}
			else
			{
				date += 24h;
			}
		}

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_record::table)
					.set(player_record::table.shield_date = date)
						.where(player_record::table.player_id == player_id)
				);
		});
	}

	void clear_shield_date(const std::uint64_t player_id)
	{
		database::access([&](database::database_t& db)
		{
			std::chrono::system_clock::time_point date{};
			db->operator()(
				sqlpp::update(player_record::table)
					.set(player_record::table.shield_date = date)
						.where(player_record::table.player_id == player_id)
				);
		});
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
			update_fob_ranking(database);
		}
	};
}

REGISTER_TABLE(database::player_records::table, -1)
