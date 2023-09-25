#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_active_sneak_mother_base : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::string& session_key) override;
	};
}
