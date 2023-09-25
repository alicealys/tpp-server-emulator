#include <std_include.hpp>

#include "cmd_get_daily_reward.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_daily_reward::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		result["count"] = 0;
		result["league_list"] = nlohmann::json::array();
		result["league_num"] = 0;
		result["personal_list"] = nlohmann::json::array();
		result["personal_num"] = 0;
		result["reward_type"] = 0;
		result["section"] = 0;

		return result;
	}
}
