#include <std_include.hpp>

#include "cmd_get_fob_event_point_exchange_params.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_get_fob_event_point_exchange_params::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
