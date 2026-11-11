#include <std_include.hpp>

#include "target_list_injury.hpp"

#include "database/models/sneak_results.hpp"
#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	target_list_t target_list_injury::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		auto list = database::sneak_results::get_sneak_results(player.get_id(), std::min(limit, 10u));
		target_list_t targets;

		const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch());

		for (auto& row : list)
		{
			target_data_t target{};

			const auto& event_data = row.get_event_data();
			const auto wormhole = database::wormholes::get_wormhole_status(player.get_id(), row.get_attacker_id());

			auto& fob_record_j = target.extra_data["owner_fob_record"];

			if (wormhole.open)
			{
				auto left_hour = 0;
				if (wormhole.expire > now)
				{
					const auto diff = wormhole.expire - now;
					left_hour = std::chrono::duration_cast<std::chrono::hours>(diff).count();
				}

				target.extra_data["owner_detail_record"]["enemy"] = 1;
				fob_record_j["left_hour"] = left_hour;
			}

			for (auto i = 0; i < 10; i++)
			{
				fob_record_j["injury_staff_count"][i] = event_data.injury_staff_count[i] + event_data.kill_staff_count[i];
				fob_record_j["capture_staff_count"][i] = event_data.capture_staff_count[i];
			}

			target.extra_data["is_win"] = static_cast<std::uint8_t>(row.is_win());
			target.extra_data["cluster"] = row.get_platform();

			event_data.capture_resource.to_json(fob_record_j["capture_resource"]);
			fob_record_j["capture_nuclear"] = event_data.capture_nuclear;
			fob_record_j["attack_gmp"] = event_data.gmp;
			fob_record_j["date_time"] = row.get_date();

			target.player_id = row.get_attacker_id();

			targets.emplace_back(target);
		}

		return targets;
	}
}
