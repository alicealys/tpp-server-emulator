#pragma once

#include "server.hpp"

namespace tpp
{
	class get_svrtime_handler : public command_handler
	{
		nlohmann::json execute(const nlohmann::json& data) override;
	};
}
