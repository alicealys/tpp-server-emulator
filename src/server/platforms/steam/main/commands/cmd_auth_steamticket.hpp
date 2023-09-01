#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_auth_steamticket : public command_handler
	{
		nlohmann::json execute(const nlohmann::json& data) override;
	};
}
