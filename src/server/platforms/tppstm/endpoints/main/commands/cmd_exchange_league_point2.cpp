#include <std_include.hpp>

#include "cmd_exchange_league_point2.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_exchange_league_point2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
