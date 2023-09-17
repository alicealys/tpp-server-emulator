#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_get_abolition_count : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::string& session_key) override;
	};
}
