#include <std_include.hpp>

#include "cmd_get_combat_deploy_result.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_combat_deploy_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["xuid"] = {};

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		result["result_list"] = nlohmann::json::array();
		result["result_num"] = 0;
		result["result"] = "NOERR";

		return result;
	}
}
