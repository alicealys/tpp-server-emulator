#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_get_informationlist2 : public command_handler
	{
		nlohmann::json execute(const nlohmann::json& data, const std::string& session_key) override;
	};
}
