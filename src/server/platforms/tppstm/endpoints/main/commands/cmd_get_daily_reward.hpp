#pragma once

#include "types/command_handler.hpp"
#include "game/game.hpp"

namespace emulator::tpp
{
	class cmd_get_daily_reward final : public command_handler
	{
	public:
		cmd_get_daily_reward();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

		static void give_daily_reward(const database::players::player& player);

	private:
		static game::reward_t& get_current_reward();

		static std::vector<game::reward_t> daily_rewards_;

	};
}
