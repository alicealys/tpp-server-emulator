#pragma once

#include "types/command_handler.hpp"

#include "../../../../tppstm/endpoints/main/commands/cmd_get_informationlist2.hpp"

namespace emulator::mgo
{
	class cmd_get_informationlist2 final : public tpp::cmd_get_informationlist2_base
	{
	public:
		cmd_get_informationlist2();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	};
}
