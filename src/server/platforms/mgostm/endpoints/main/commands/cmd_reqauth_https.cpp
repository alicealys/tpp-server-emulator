#include <std_include.hpp>

#include "cmd_reqauth_https.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_reqauth_https::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
