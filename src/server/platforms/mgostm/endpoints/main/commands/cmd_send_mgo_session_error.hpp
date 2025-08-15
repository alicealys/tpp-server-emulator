#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_send_mgo_session_error final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
