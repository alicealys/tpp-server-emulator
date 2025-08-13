#include <std_include.hpp>

#include "cmd_mgo_dlc_update.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_mgo_dlc_update::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["now_dlc_flags"] = 271;
		result["old_dlc_flags"] = 271;

		return result;
	}
}
