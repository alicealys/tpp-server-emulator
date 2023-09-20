#include <std_include.hpp>

#include "cmd_get_challenge_task_rewards.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_challenge_task_rewards::cmd_get_challenge_task_rewards()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_CHALLENGE_TASK_REWARDS));
	}

	nlohmann::json cmd_get_challenge_task_rewards::execute(nlohmann::json& data, const std::string& session_key)
	{
		return this->list_;
	}
}
