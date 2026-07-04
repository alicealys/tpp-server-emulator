#include <std_include.hpp>

#include "cmd_get_platform_construction_progress.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_platform_construction_progress::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["progress"] = nlohmann::json::array();

		return result;
	}
}
