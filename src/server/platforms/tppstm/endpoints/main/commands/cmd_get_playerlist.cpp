#include <std_include.hpp>

#include "cmd_get_playerlist.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"
#include "database/models/player_data.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_playerlist::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["xuid"] = {};

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto stats = database::player_records::find(player->get_id());
		const auto player_data = database::player_data::find(player->get_id());

		result["player_num"] = 0;

		if (stats.has_value() && player_data.get())
		{
			result["player_num"] = 1;

			result["player_list"][0]["index"] = 0;
			result["player_list"][0]["name"] = std::format("{}_player01", player->get_account_id());

			result["player_list"][0]["espionage_lose"] = stats->get_sneak_win();
			result["player_list"][0]["espionage_win"] = stats->get_sneak_lose();
			result["player_list"][0]["fob_grade"] = stats->get_fob_grade();
			result["player_list"][0]["fob_point"] = stats->get_fob_point();
			result["player_list"][0]["fob_rank"] = stats->get_fob_rank();
			result["player_list"][0]["is_insurance"] = stats->get_is_insurance();
			result["player_list"][0]["league_grade"] = stats->get_league_grade();
			result["player_list"][0]["league_rank"] = stats->get_league_rank();
			result["player_list"][0]["playtime"] = stats->get_playtime();
			result["player_list"][0]["point"] = stats->get_point();
		}

		result["result"] = "NOERR";

		return result;
	}
}
