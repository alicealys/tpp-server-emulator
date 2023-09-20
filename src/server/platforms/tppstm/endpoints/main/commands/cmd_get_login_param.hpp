#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_get_login_param : public command_handler
	{
	public:
		cmd_get_login_param();
		nlohmann::json execute(nlohmann::json& data, const std::string& session_key) override;

	private:
		nlohmann::json list_;

	};
}
