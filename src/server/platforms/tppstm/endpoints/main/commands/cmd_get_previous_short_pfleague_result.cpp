#include <std_include.hpp>

#include "cmd_get_previous_short_pfleague_result.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_get_previous_short_pfleague_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
