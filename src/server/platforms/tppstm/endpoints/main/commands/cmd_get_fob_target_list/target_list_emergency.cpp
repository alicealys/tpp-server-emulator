#include <std_include.hpp>

#include "target_list_emergency.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	target_list_t target_list_emergency::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto search_intruder = [&](const std::uint64_t owner_id, const bool follow)
		{
			const auto active_sneak = database::players::find_active_sneak(owner_id, true, true);
			if (!active_sneak.has_value())
			{
				return;
			}

			target_data_t target;

			target.player_id = owner_id;

			const auto attacker = database::players::find(active_sneak->get_player_id());
			const auto attacker_record = database::player_records::find(active_sneak->get_player_id());
			const auto attacker_data = database::player_data::find(active_sneak->get_player_id());
			const auto fob_list = database::fobs::get_fob_list(active_sneak->get_owner_id());

			for (auto i = 0ull; i < fob_list.size(); i++)
			{
				auto& fob = fob_list[i];
				auto& param_j = target.extra_data["mother_base_param"][i + 1];
				param_j["area_id"] = 0;
				param_j["construct_param"] = fob.get_construct_param().packed;
				param_j["fob_index"] = fob.get_index();
				param_j["mother_base_id"] = fob.get_id();
				param_j["platform_count"] = fob.get_platform_count();
				param_j["price"] = 0;
				param_j["security_rank"] = fob.get_security_rank();

				if (i == 0)
				{
					target.extra_data["mother_base_param"][0] = target.extra_data["mother_base_param"][1];
				}

				if (fob.get_id() == active_sneak->get_fob_id())
				{
					target.extra_data["mother_base_param"][0] = target.extra_data["mother_base_param"][i + 1];
				}
			}

			game::emblem_t emblem{};
			attacker_data->get_emblem(emblem);

			emblem.to_json(target.extra_data["attacker_emblem"]);
			auto& attacker_espionage_j = target.extra_data["attacker_espionage"];
			attacker_espionage_j["win"] = attacker_record->get_sneak_win();
			attacker_espionage_j["lose"] = attacker_record->get_sneak_lose();
			attacker_espionage_j["score"] = attacker_record->get_fob_point();
			attacker_espionage_j["section"] = 0;

			auto& attacker_info_j = target.extra_data["attacker_info"];
			attacker_info_j["player_id"] = attacker->get_id();
			attacker_info_j["player_name"] = std::format("{}_player01", attacker->get_account_id());
			attacker_info_j["xuid"] = attacker->get_id();

			target.extra_data["follow"] = std::uint8_t(follow);
			target.extra_data["sneak_mode"] = active_sneak->get_game_sneak_mode();
			target.extra_data["attacker_sneak_rank_grade"] = attacker_record->get_fob_grade();

			targets.emplace_back(target);
		};

		search_intruder(player.get_id(), false);

		const auto follows = database::player_follows::get_follows(player.get_id());
		for (const auto& follow_id : follows)
		{
			search_intruder(follow_id, true);
		}

		return targets;
	}
}
