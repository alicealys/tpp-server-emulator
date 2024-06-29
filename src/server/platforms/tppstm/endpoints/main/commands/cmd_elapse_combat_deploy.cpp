#include <std_include.hpp>

#include "cmd_elapse_combat_deploy.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_elapse_combat_deploy::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
