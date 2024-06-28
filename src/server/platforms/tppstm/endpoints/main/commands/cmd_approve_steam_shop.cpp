#include <std_include.hpp>

#include "cmd_approve_steam_shop.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_approve_steam_shop::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
