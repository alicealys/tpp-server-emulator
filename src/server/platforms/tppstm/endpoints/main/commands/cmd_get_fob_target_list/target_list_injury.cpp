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

			auto& sneak_data = row.get_data();

			const auto wormhole = database::wormholes::get_wormhole_status(player.get_id(), row.get_player_id());

			if (wormhole.open)
			{
				auto left_hour = 0;
				if (wormhole.expire > now)
				{
					const auto diff = wormhole.expire - now;
					left_hour = std::chrono::duration_cast<std::chrono::hours>(diff).count();
				}

				target.extra_data["owner_detail_record"]["enemy"] = 1;
				target.extra_data["owner_fob_record"]["left_hour"] = left_hour;
			}

			target.extra_data["owner_fob_record"]["injury_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			target.extra_data["is_win"] = static_cast<int>(row.is_win());
			target.extra_data["cluster"] = row.get_platform();

			for (auto i = 0ull; i < sneak_data["injure_soldier_id"].size(); i++)
			{
				const auto header_val = sneak_data["injure_soldier_id"][i]["param"][0].get<std::uint32_t>();
				database::player_data::staff_header_t header{};

				std::memcpy(&header, &header_val, sizeof(database::player_data::staff_header_t));
				auto& value = target.extra_data["owner_fob_record"]["injury_staff_count"][header.peak_rank];

				const auto current = value.get<std::uint32_t>();
				value = current + 1;
			}

			target.extra_data["owner_fob_record"]["date_time"] = row.get_date();
			target.player_id = row.get_player_id();

			targets.emplace_back(target);
		}

		return targets;
	}
}
