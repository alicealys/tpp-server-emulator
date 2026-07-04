#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	using reward_handler_t = std::function<void(const std::uint64_t, const std::uint32_t)>;

	struct reward_t
	{
		std::uint32_t type;
		std::optional<reward_handler_t> handler;
	};

	struct daily_reward_t
	{
		std::uint32_t count;
		reward_t instance;
	};

	class cmd_get_daily_reward final : public command_handler
	{
	public:
		cmd_get_daily_reward();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

		static void give_daily_reward(const database::players::player& player);
		static void give_reward(const std::uint64_t player_id, const std::uint32_t type, const std::uint32_t count);

	private:
		void register_reward(const std::string& name, const std::uint32_t type, const std::optional<reward_handler_t>& handler = {});
		static daily_reward_t& get_current_reward();

		static std::vector<daily_reward_t> daily_rewards_;
		static std::unordered_map<std::string, reward_t> reward_map_;
		static std::unordered_map<std::uint32_t, reward_t> reward_map_int_;

	};
}
