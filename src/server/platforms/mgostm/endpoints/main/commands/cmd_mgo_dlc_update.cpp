#include <std_include.hpp>

#include "cmd_mgo_dlc_update.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_mgo_dlc_update::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
