#include <std_include.hpp>

#include "player_stats.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#define TABLE_DEF R"(
create table if not exists `player_stats`
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
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
))"

namespace database::player_stats
{
	auto player_stats_table = player_stats_table_t();

	std::optional<stats> find(const std::uint64_t player_id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(player_stats_table))
					.from(player_stats_table)
						.where(player_stats_table.player_id == player_id));

		if (results.empty())
		{
			return {};
		}

		const auto& row = results.front();
		return {stats(row)};
	}

	stats find_or_insert(const std::uint64_t player_id)
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
			sqlpp::insert_into(player_stats_table)
				.set(player_stats_table.player_id = player_id));
#pragma warning(pop)

		const auto found = find(player_id);
		if (!found.has_value())
		{
			throw std::runtime_error("[database::player_stats::insert] Insertion failed");
		}

		return found.value();
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

REGISTER_TABLE(database::player_stats::table, -1)
