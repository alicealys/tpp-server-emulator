#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_get_mgo_title_usr final : public command_handler
	{
	public:
		static nlohmann::json generate(nlohmann::json& data, const std::optional<database::players::player>& player);

		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player();
	};
}
