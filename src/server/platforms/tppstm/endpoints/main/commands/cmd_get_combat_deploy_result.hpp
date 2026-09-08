#pragma once

#include "types/command_handler.hpp"
#include "database/models/combat_deployments.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	class cmd_get_combat_deploy_result final : public command_handler
	{
	public:
		static void give_reward(const std::uint64_t player_id, const database::combat_deployments::mission_reward_t& reward);
		static bool can_give_reward(const database::player_data::player_data& player_data, const database::combat_deployments::mission_reward_t& reward);

		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;
	};
}
