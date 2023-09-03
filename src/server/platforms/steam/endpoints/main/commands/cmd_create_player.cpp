#include <std_include.hpp>

#include "cmd_create_player.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_stats.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_create_player::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["xuid"] = {};

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		database::player_stats::find_or_insert(player->get_id());
		result["player_id"] = player->get_id();
		result["result"] = "NOERR";

		return result;
	}
}
