#include <std_include.hpp>

#include "target_list_pickup.hpp"
#include "target_list_follow.hpp"

#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	target_list_t target_list_pickup::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		if (database::vars.pvp_mode)
		{
			return target_list_follow::generate_impl(player, player_data, limit);
		}

		const auto list = database::player_records::find_same_grade_players(player.get_id(), std::min(limit, 10u));
		target_list_t targets;

		for (const auto& row : list)
		{
			target_data_t target{};
			target.player_id = row.get_player_id();
			target.player_record.emplace(row);
			targets.emplace_back(target);
		}

		return targets;
	}
}
