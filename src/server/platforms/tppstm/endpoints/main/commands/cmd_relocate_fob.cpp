#include <std_include.hpp>

#include "cmd_relocate_fob.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_relocate_fob::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
