#include <std_include.hpp>

#include "cmd_get_platform_construction_progress.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_get_platform_construction_progress::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
