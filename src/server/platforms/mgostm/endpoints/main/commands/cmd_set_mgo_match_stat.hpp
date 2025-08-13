#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_set_mgo_match_stat final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
