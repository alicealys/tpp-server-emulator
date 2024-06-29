#include <std_include.hpp>

#include "cmd_get_ranking.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_get_ranking::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
