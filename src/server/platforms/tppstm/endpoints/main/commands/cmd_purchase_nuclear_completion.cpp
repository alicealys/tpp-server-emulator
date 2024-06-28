#include <std_include.hpp>

#include "cmd_purchase_nuclear_completion.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_purchase_nuclear_completion::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
