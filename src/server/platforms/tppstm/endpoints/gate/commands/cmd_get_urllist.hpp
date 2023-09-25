#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_get_urllist : public command_handler
	{
	public:
		cmd_get_urllist();

		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
