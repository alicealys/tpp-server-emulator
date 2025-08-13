#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_check_short_pfleague_enterable final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
