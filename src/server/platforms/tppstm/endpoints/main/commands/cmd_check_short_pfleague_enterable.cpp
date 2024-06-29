#include <std_include.hpp>

#include "cmd_check_short_pfleague_enterable.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_check_short_pfleague_enterable::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
