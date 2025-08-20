#include <std_include.hpp>

#include "cmd_send_mission_result.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_send_mission_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return {};
	}
}
