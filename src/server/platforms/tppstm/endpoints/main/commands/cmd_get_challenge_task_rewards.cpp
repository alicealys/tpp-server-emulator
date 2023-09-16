#include <std_include.hpp>

#include "cmd_get_challenge_task_rewards.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_challenge_task_rewards::execute(const nlohmann::json& data, const std::string& session_key)
	{
		static const auto list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_CHALLENGE_TASK_REWARDS));

		nlohmann::json result = list;

		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}
