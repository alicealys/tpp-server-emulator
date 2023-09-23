#include <std_include.hpp>

#include "cmd_get_league_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_league_result::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		result["info"]["begin"] = 0;
		result["info"]["current"] = 0;
		result["info"]["day_battle"] = 0;
		result["info"]["end"] = 0;
		result["info"]["next"] = 0;
		result["info"]["history_count"] = 0;
		result["info"]["player_count"] = 0;
		result["info"]["point"] = 0;
		result["info"]["section"] = 0;
		result["info"]["match_history"] = nlohmann::json::array();
		result["info"]["player_count"] = nlohmann::json::array();
		result["result"] = "ERR_NOTIMPLEMENT";

		return result;
	}
}
