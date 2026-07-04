#include <std_include.hpp>

#include "cmd_send_troops.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_send_troops::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return {};
	}
}
