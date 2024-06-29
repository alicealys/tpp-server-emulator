#include <std_include.hpp>

#include "cmd_use_pf_item.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_use_pf_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
