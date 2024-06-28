#include <std_include.hpp>

#include "cmd_destruct_online_nuclear.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_destruct_online_nuclear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
