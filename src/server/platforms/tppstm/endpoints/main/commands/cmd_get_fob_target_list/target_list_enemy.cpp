#include <std_include.hpp>

#include "target_list_enemy.hpp"

#include "database/models/wormholes.hpp"
#include "database/models/sneak_results.hpp"

namespace emulator::tpp
{
	target_list_t target_list_enemy::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		const auto list = database::wormholes::get_wormhole_list(player.get_id());
		target_list_t targets;

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());

		for (const auto& wormhole : list)
		{
			if (!wormhole.is_open())
			{
				continue;
			}

			target_data_t target{};
			target.player_id = wormhole.opponent_id;
			target.player_record = database::player_records::find(wormhole.opponent_id);

			const auto is_restricted = !wormhole.first && target.player_record->get_shield_date() > now.count();

			target.extra_data["is_sneak_restriction"] = std::uint8_t(is_restricted);
			target.extra_data["owner_detail_record"]["enemy"] = 1;

			const auto damage = database::sneak_results::get_damage_from(player.get_id(),
				wormhole.opponent_id, wormhole.first_date);

			auto& fob_record_j = target.extra_data["owner_fob_record"];
			fob_record_j["capture_staff"] = damage.capture_staff_count;
			fob_record_j["capture_resource_count"] = damage.capture_resource_count;
			fob_record_j["attack_gmp"] = damage.attack_gmp;
			fob_record_j["attack_count"] = damage.attack_count;
			fob_record_j["left_hour"] = wormhole.get_left_hour();

			targets.emplace_back(target);
		}

		return targets;
	}
}
