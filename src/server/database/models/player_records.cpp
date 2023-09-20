#include <std_include.hpp>

#include "player_records.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#define TABLE_DEF R"(
create table if not exists `player_record`
(
	id							bigint unsigned	not null	auto_increment,
	player_id					bigint unsigned	not null,
	fob_grade					int				default 0,
	fob_point					int				default 0,
	fob_rank					int				default 0,
	is_insurance				boolean			default false,
	league_grade				int				default 0,
	league_rank					int				default 0,
	playtime					int				default 0,
	point						int				default 0,
	fob_defense_win				int				default 0,
	fob_defense_lose			int				default 0,
	fob_sneak_win				int				default 0,
	fob_sneak_lose				int				default 0,
	fob_deploy_emergency_count	int				default 0,
	shield_date					datetime not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
))"

namespace database::player_records
{
	auto player_records_table = player_records_table_t();

	std::optional<player_record> find(const std::uint64_t player_id)
	{
		auto results = database::get()->operator()(
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

#pragma warning(push)
#pragma warning(disable: 4127)
		database::get()->operator()(
			sqlpp::insert_into(player_records_table)
				.set(player_records_table.player_id = player_id,
					 player_records_table.shield_date = std::chrono::system_clock::now()));
#pragma warning(pop)

		const auto found = find(player_id);
		if (!found.has_value())
		{
			throw std::runtime_error("[database::player_records::insert] Insertion failed");
		}

		return found.value();
	}

	void add_sneak_result(const std::uint64_t player_id, const std::int32_t point_add, const bool is_win)
	{
		database::get()->operator()(
			sqlpp::update(player_records_table)
				.set(player_records_table.fob_point = player_records_table.fob_point + point_add,
					 player_records_table.fob_sneak_win = player_records_table.fob_sneak_win 
						+ static_cast<std::int32_t>(is_win),
					 player_records_table.fob_sneak_lose = player_records_table.fob_sneak_lose 
						+ static_cast<std::int32_t>(!is_win))
						.where(player_records_table.player_id == player_id)
			);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}
	};
}

REGISTER_TABLE(database::player_records::table, -1)
