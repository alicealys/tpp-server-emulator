#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_sync_loadout : public command_handler
	{
		nlohmann::json execute(const nlohmann::json& data, const std::string& session_key) override;
	};
}
