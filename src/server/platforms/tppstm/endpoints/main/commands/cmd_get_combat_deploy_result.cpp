#include <std_include.hpp>

#include "cmd_get_combat_deploy_result.hpp"

#include "database/models/players.hpp"
#include "database/models/player_stats.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_combat_deploy_result::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["xuid"] = {};

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		result["result_list"] = nlohmann::json::array();
		result["result_num"] = 0;
		result["result"] = "NOERR";

		return result;
	}
}
