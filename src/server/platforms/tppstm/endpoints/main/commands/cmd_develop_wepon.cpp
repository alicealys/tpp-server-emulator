#include <std_include.hpp>

#include "cmd_develop_wepon.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_develop_wepon::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
