#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_use_pf_item final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
