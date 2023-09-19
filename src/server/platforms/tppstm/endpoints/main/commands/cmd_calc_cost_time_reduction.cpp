#include <std_include.hpp>

#include "cmd_calc_cost_time_reduction.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_calc_cost_time_reduction::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		const auto& kind_j = data["kind"];
		const auto& remaining_time_j = data["remaining_time"];

		if (!kind_j.is_string() || !remaining_time_j.is_number_integer())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		return result;
	}
}
