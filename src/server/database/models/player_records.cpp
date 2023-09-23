#include <std_include.hpp>

#include "player_records.hpp"

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
	player_records_table_t player_records_table;

	std::optional<player_record> find(const std::uint64_t player_id)
	{
		return database::access<std::optional<player_record>>([&](database::database_t& db)
			-> std::optional<player_record>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(player_records_table))
						.from(player_records_table)
							.where(player_records_table.player_id == player_id));

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
				sqlpp::insert_into(player_records_table)
					.set(player_records_table.player_id = player_id,
						 player_records_table.shield_date = std::chrono::system_clock::now()));
		});

		const auto found = find(player_id);
		if (!found.has_value())
		{
			throw std::runtime_error("[database::player_records::insert] Insertion failed");
		}

		return found.value();
	}

	void add_sneak_result(const std::uint64_t player_id, const std::int32_t point_add, const bool is_win)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_records_table)
					.set(player_records_table.fob_point = player_records_table.fob_point + point_add,
						 player_records_table.fob_sneak_win = player_records_table.fob_sneak_win 
							+ static_cast<std::int32_t>(is_win),
						 player_records_table.fob_sneak_lose = player_records_table.fob_sneak_lose 
							+ static_cast<std::int32_t>(!is_win))
							.where(player_records_table.player_id == player_id)
				);
		});
	}

	void sync_prev_values(const std::uint64_t player_id)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(player_records_table)
					.set(player_records_table.prev_fob_grade = player_records_table.fob_grade,
						 player_records_table.prev_fob_rank = player_records_table.fob_rank)
							.where(player_records_table.player_id == player_id)
				);
		});
	}

	void update_fob_ranking()
	{
		database::access([&](database::database_t& db)
		{
			db->execute("set @r=0;");
			db->execute("update player_records set fob_rank= @r:= (@r + 1) where fob_point > 0 order by fob_point desc;");

			static std::vector<std::pair<std::uint32_t, std::uint32_t>> rank_ranges =
			{
				{0, 0},				// 0
				{100, 200},			// 1
				{200, 400},			// 2
				{400, 800},			// 3
				{800, 1500},		// 4
				{1500, 3000},		// 5
				{3000, 4000},		// 6
				{4000, 7000},		// 7
				{7000, 10000},		// 8
				{10000, 50000},		// 9
				{50000, 500000},	// 9
				{500000, 0},		// 9
			};

			for (auto i = 0; i < rank_ranges.size(); i++)
			{
				const auto& range = rank_ranges[i];
				if (i == rank_ranges.size() - 1)
				{
					db->operator()(
						sqlpp::update(player_records_table)
							.set(player_records_table.fob_grade = i)
								.where(player_records_table.fob_point >= range.first)
						);
				}
				else
				{
					db->operator()(
						sqlpp::update(player_records_table)
							.set(player_records_table.fob_grade = i)
								.where(player_records_table.fob_point >= range.first && player_records_table.fob_point < range.second)
						);
				}
			}
		});
	}

	std::vector<player_record> find_players_of_rank(const std::uint64_t player_id, const std::uint32_t rank ,const std::uint32_t limit)
	{
		return database::access<std::vector<player_record>>([&](database::database_t& db)
			-> std::vector<player_record>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(player_records_table))
						.from(player_records_table)
							.where(player_records_table.fob_rank == rank)
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

	std::vector<player_record> find_same_rank_players(const std::uint64_t player_id, const std::uint32_t limit)
	{
		return database::access<std::vector<player_record>>([&](database::database_t& db)
			-> std::vector<player_record>
		{
			const auto record = find(player_id);
			if (!record.has_value())
			{
				return {};
			}

			return find_players_of_rank(player_id, record->get_fob_rank(), limit);
		});
	}

	std::vector<player_record> find_higher_rank_players(const std::uint64_t player_id, const std::uint32_t limit)
	{
		return database::access<std::vector<player_record>>([&](database::database_t& db)
			-> std::vector<player_record>
		{
			const auto record = find(player_id);
			if (!record.has_value())
			{
				return {};
			}

			const auto rank = record->get_fob_rank();
			const auto higher_rank = rank == highest_rank ? rank : rank + 1;

			return find_players_of_rank(player_id, higher_rank, limit);
		});
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
			update_fob_ranking();
		}
	};
}

REGISTER_TABLE(database::player_records::table, -1)
