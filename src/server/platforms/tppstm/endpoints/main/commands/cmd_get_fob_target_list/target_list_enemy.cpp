#include <std_include.hpp>

#include "target_list_enemy.hpp"

#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	target_list_t target_list_enemy::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		const auto list = database::wormholes::get_wormhole_list(player.get_id());
		target_list_t targets;

		for (const auto& wormhole : list)
		{
			if (!wormhole.is_open())
			{
				continue;
			}

			target_data_t target{};
			target.extra_data["owner_detail_record"]["enemy"] = 1;
			target.extra_data["owner_fob_record"]["attack_count"] = wormhole.attack_count;
			target.extra_data["owner_fob_record"]["left_hour"] = wormhole.get_left_hour();
			target.player_id = wormhole.opponent_id;
			targets.emplace_back(target);
		}

		return targets;
	}
}
