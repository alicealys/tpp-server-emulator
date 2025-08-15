#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_purchase_mgo_gear_color final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
