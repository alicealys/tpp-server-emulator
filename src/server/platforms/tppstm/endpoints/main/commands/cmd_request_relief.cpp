#include <std_include.hpp>

#include "cmd_request_relief.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_request_relief::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return {};
	}
}
