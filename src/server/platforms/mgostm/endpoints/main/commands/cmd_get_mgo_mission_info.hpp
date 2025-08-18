#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_get_mgo_mission_info final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;
	};
}
