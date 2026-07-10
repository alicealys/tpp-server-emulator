#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_destruct_online_nuclear final : public command_handler
	{
	public:
		cmd_destruct_online_nuclear();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;
	};
}
