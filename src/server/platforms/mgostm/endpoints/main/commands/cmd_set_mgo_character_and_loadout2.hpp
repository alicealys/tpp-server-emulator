#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_set_mgo_character_and_loadout2 final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
