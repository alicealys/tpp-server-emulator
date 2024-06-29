#include <std_include.hpp>

#include "cmd_delete_follow.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_delete_follow::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
