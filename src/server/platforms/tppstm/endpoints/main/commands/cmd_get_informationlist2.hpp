#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_get_informationlist2 : public command_handler
	{
	public:
		cmd_get_informationlist2();
		nlohmann::json execute(nlohmann::json& data, const std::string& session_key) override;

	private:
		nlohmann::json list_;

	};
}
