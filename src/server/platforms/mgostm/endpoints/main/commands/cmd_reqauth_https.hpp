#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_reqauth_https final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
