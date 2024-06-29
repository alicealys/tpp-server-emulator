#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	nlohmann::json create_nuclear(const std::optional<database::players::player>& player);

	class cmd_create_nuclear : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;
	};
}
