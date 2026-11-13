#include <std_include.hpp>

#include "database/models/player_follows.hpp"
#include "database/models/player_records.hpp"
#include "database/models/wormholes.hpp"

#include "target_list_fr_enemy.hpp"
#include "types/command_handler.hpp"

namespace emulator::tpp
{
	target_list_t target_list_fr_enemy::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto follows = database::player_follows::get_follows(player.get_id());

		for (const auto& follow : follows)
		{
			const auto friend_wormholes = database::wormholes::get_wormhole_list(follow.get_player_id(), database::wormholes::wormhole_flag_invalid, player.get_id());
			for (const auto& wormhole : friend_wormholes)
			{
				if (!wormhole.is_open(true) || wormhole.opponent_id == player.get_id())
				{
					continue;
				}

				target_data_t target{};
				target.extra_data["owner_detail_record"]["enemy"] = 1;
				target.extra_data["owner_fob_record"]["left_hour"] = wormhole.get_left_hour();
				target.extra_data["owner_fob_record"]["attack_count"] = wormhole.attack_count;
				target.extra_data["attacker_info"] = player_info(follow.player);
				target.extra_data["attacker_sneak_rank_grade"] = follow.record.get_fob_grade();
				target.player_id = wormhole.opponent_id;
				targets.emplace_back(target);
			}
		}

		return targets;
	}
}
