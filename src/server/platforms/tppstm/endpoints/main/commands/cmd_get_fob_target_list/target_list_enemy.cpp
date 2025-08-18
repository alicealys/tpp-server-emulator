#include <std_include.hpp>

#include "target_list_enemy.hpp"

#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	target_list_t target_list_enemy::generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit)
	{
		auto list = database::wormholes::find_active_wormholes(player.get_id());
		target_list_t targets;

		const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch());

		for (auto& [to_player_id, wormhole] : list)
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

			target.player_id = wormhole.to_player_id;

			targets.emplace_back(target);
		}

		return targets;
	}
}
