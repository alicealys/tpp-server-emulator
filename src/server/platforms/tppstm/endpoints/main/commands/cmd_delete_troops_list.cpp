#include <std_include.hpp>

#include "cmd_delete_troops_list.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_delete_troops_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
