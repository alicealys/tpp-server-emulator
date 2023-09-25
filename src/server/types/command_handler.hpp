#pragma once

#include "database/models/players.hpp"

namespace tpp
{
	class command_handler
	{
	public:
		virtual nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player)
		{
			throw std::runtime_error("unimplemented command");
		};
	};
}
