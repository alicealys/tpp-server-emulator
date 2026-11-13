#include <std_include.hpp>

#include "target_list_injury.hpp"

#include "database/models/sneak_results.hpp"
#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	target_list_t target_list_injury::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		const auto list = database::sneak_results::get_sneak_results(player.get_id(), std::min(limit, 10u));
		target_list_t targets;

		const auto wormholes = database::wormholes::get_wormhole_list(player.get_id());

		for (const auto& sneak_result : list)
		{
			target_data_t target{};

			const auto& event_data = sneak_result.get_event_data();

			auto& fob_record_j = target.extra_data["owner_fob_record"];
			const auto iter = std::ranges::find_if(wormholes.begin(), wormholes.end(), 
				[&](const database::wormholes::wormhole_status_t& status)
				{
					return status.opponent_id == sneak_result.get_attacker_id();
				}
			);

			if (iter != wormholes.end() && iter->is_open())
			{
				target.extra_data["owner_detail_record"]["enemy"] = 1;
				fob_record_j["left_hour"] = iter->get_left_hour();
			}

			for (auto i = 0; i < 10; i++)
			{
				fob_record_j["injury_staff_count"][i] = event_data.injury_staff_count[i] + event_data.kill_staff_count[i];
				fob_record_j["capture_staff_count"][i] = event_data.capture_staff_count[i];
			}

			target.extra_data["is_win"] = static_cast<std::uint8_t>(sneak_result.is_win());
			target.extra_data["cluster"] = sneak_result.get_cluster();

			event_data.capture_resource.to_json(fob_record_j["capture_resource"]);
			fob_record_j["capture_nuclear"] = event_data.capture_nuclear;
			fob_record_j["attack_gmp"] = event_data.gmp;
			fob_record_j["date_time"] = sneak_result.get_date();

			target.player_id = sneak_result.get_attacker_id();

			targets.emplace_back(target);
		}

		return targets;
	}
}
