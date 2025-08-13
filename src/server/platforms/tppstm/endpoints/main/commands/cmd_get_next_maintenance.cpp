#include <std_include.hpp>

#include "cmd_get_next_maintenance.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_next_maintenance::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["maintenance_type"] = 0;
		result["message_type"] = 0;
		result["next_maintenance"] = std::numeric_limits<std::int32_t>::max();

		return result;
	}
}
