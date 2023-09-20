#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_mining_resource : public command_handler
	{
	public:
		cmd_mining_resource();
		nlohmann::json execute(nlohmann::json& data, const std::string& session_key) override;

	private:
		nlohmann::json list_;

	};
}
