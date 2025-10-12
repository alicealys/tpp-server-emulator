#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_set_mgo_character2 final : public command_handler
	{
	public:
		static bool set_character(nlohmann::json& data, const std::optional<database::players::player>& player);
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
