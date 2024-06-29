#include <std_include.hpp>

#include "cmd_request_relief.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_request_relief::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
