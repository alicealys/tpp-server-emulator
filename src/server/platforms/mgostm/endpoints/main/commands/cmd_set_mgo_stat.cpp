#include <std_include.hpp>

#include "cmd_set_mgo_stat.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
