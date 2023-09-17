#pragma once

#include "types/command_handler.hpp"

#include "../main_handler.hpp"

namespace tpp
{
	class cmd_develop_server_item : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::string& session_key) override;
	};
}
