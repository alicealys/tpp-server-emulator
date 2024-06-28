#include <std_include.hpp>

#include "cmd_get_fob_reward_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_reward_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		result["reward_count"] = 0;
		result["version"] = 0;
		result["reward_info"] = nlohmann::json::array();

		return result;
	}
}
