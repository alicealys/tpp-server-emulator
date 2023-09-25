#include <std_include.hpp>

#include "cmd_sync_loadout.hpp"

#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_loadout::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		if (!data["loadout"].is_object())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		database::player_data::sync_loadout(player->get_id(), data["loadout"]);

		return result;
	}
}
