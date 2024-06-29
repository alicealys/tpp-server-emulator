#include <std_include.hpp>

#include "cmd_reset_mother_base.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_reset_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
