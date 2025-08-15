#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_get_mgo_title_usr2 final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
