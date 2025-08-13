#include <std_include.hpp>

#include "cmd_gdpr_check.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_gdpr_check::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
