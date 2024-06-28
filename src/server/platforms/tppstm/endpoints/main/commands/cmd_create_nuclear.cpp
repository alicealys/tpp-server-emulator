#include <std_include.hpp>

#include "cmd_create_nuclear.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_create_nuclear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
