#include <std_include.hpp>

#include "cmd_get_mgo_user_data.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_user_data::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
