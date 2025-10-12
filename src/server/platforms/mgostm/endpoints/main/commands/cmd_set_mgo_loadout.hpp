#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_set_mgo_loadout final : public command_handler
	{
	public:
		static bool set_loadout(nlohmann::json& data, const std::optional<database::players::player>& player);
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;
	};
}
