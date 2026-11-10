#include <std_include.hpp>

#include "cmd_get_fob_status.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_status::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		result["fob_index"] = -1;
		result["is_rescue"] = 0;
		result["is_reward"] = 0;
		result["kill_count"] = 0;
		result["sneak_mode"] = -1;

		const auto set_sneak = [&](const database::players::sneak_info& sneak, const bool is_support)
		{
			const auto fob = database::fobs::get_fob(sneak.get_fob_id());
			result["is_rescue"] = 1;
			result["fob_index"] = fob->get_index();
			result["sneak_mode"] = sneak.get_game_sneak_mode();
		};

		const auto self_sneak = database::players::find_active_sneak(player->get_id(), true, true);
		if (self_sneak.has_value())
		{
			set_sneak(self_sneak.value(), false);
			return result;
		}

		const auto support_sneak = database::players::find_active_sneak_to_supporting(player->get_id(), true, true);
		if (support_sneak.has_value())
		{
			set_sneak(support_sneak.value(), false);
			return result;
		}

		const auto active_sneak = database::players::find_active_sneak(player->get_id(), true, true);

		auto& record_j = result["record"];
		record_j["defense"]["win"] = stats->get_defense_win();
		record_j["defense"]["lose"] = stats->get_defense_lose();
		record_j["insurance"] = 0;
		record_j["score"] = stats->get_fob_point();
		record_j["shield_date"] = stats->get_shield_date();
		record_j["sneak"]["win"] = stats->get_sneak_win();
		record_j["sneak"]["lose"] = stats->get_sneak_lose();

		return result;
	}

	bool cmd_get_fob_status::needs_player()
	{
		return true;
	}
}
