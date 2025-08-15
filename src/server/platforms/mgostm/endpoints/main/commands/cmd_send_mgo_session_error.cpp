#include <std_include.hpp>

#include "cmd_send_mgo_session_error.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_send_mgo_session_error::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
