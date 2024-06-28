#include <std_include.hpp>

#include "cmd_get_fob_status.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_status::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto active_sneak = database::players::find_active_sneak(player->get_id(), true, true);

		result["result"] = "NOERR";
		result["fob_index"] = -1;
		result["is_rescue"] = static_cast<int>(active_sneak.has_value());
		result["is_reward"] = 0;
		result["kill_count"] = 0;
		result["sneak_mode"] = -1;

		result["record"]["defense"]["win"] = stats->get_defense_win();
		result["record"]["defense"]["lose"] = stats->get_defense_lose();
		result["record"]["insurance"] = 0;
		result["record"]["score"] = stats->get_fob_point();
		result["record"]["shield_date"] = stats->get_shield_date();
		result["record"]["sneak"]["win"] = stats->get_sneak_win();
		result["record"]["sneak"]["lose"] = stats->get_sneak_lose();

		return result;
	}
}
