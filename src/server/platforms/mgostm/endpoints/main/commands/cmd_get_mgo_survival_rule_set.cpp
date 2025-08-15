#include <std_include.hpp>

#include "cmd_get_mgo_survival_rule_set.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_survival_rule_set::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
