#include <std_include.hpp>

#include "cmd_get_challenge_task_rewards.hpp"

namespace emulator::tpp
{
	cmd_get_challenge_task_rewards::cmd_get_challenge_task_rewards()
	{
		this->list_ = resource(RESOURCE_CHALLENGE_TASK_REWARDS);
	}

	nlohmann::json cmd_get_challenge_task_rewards::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
