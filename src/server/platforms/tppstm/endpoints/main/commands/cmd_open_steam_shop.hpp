#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_open_steam_shop : public command_handler
	{
	public:
		cmd_open_steam_shop();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

	private:
		nlohmann::json list_;

	};
}
