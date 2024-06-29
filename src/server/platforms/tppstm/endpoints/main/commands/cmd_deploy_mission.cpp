#include <std_include.hpp>

#include "cmd_deploy_mission.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_deploy_mission::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
