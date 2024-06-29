#include <std_include.hpp>

#include "cmd_exchange_fob_event_point.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_exchange_fob_event_point::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
