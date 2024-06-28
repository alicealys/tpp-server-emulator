#include <std_include.hpp>

#include "cmd_send_online_challenge_task_status.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_send_online_challenge_task_status::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
