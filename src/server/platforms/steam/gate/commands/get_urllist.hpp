#pragma once

#include "server.hpp"

namespace tpp
{
	class get_urllist_handler : public command_handler
	{
		nlohmann::json execute(const nlohmann::json& data) override;
	};
}
