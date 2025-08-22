#include <std_include.hpp>

#include "database/models/player_follows.hpp"
#include "database/models/wormholes.hpp"

#include "target_list_fr_enemy.hpp"
#include "types/command_handler.hpp"

namespace emulator::tpp
{
	target_list_t target_list_fr_enemy::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto follows = database::player_follows::get_follows(player.get_id());

		for (const auto& follow_id : follows)
		{
			const auto follow = database::players::find(follow_id);
			if (!follow.has_value())
			{
				continue;
			}

			const auto list = database::wormholes::get_wormholes_status(follow_id);

			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			for (const auto& wormhole : list)
			{
				target_data_t target{};

				auto left_hour = 0;
				if (wormhole.expire > now)
				{
					const auto diff = wormhole.expire - now;
					left_hour = std::chrono::duration_cast<std::chrono::hours>(diff).count();
				}

				target.extra_data["owner_detail_record"]["enemy"] = 1;
				target.extra_data["owner_fob_record"]["left_hour"] = left_hour;
				target.extra_data["attacker_info"] = player_info(follow.value());

				target.player_id = wormhole.to_player_id;

				targets.emplace_back(target);
			}
		}

		return targets;
	}
}
