#include <std_include.hpp>

#include "cmd_send_heartbeat.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_send_heartbeat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
