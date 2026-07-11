#include <std_include.hpp>

#include "cmd_get_mgo_survival_rule_set.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_survival_rule_set::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["survival_rule_set"]["ruleset_no"] = 0;
		result["survival_rule_set"]["map"] = 0;
		result["survival_rule_set"]["rule"] = 2;
		result["survival_rule_set"]["rush"] = 0;
		result["survival_rule_set"]["night"] = 0;
		result["survival_rule_set"]["time"] = 15;
		result["survival_rule_set"]["unique_character"] = 0;
		result["survival_rule_set"]["ticket"] = 50;
		result["survival_rule_set"]["weather"] = 1;
		result["survival_rule_set"]["flags"] = 0;

		return result;
	}
}
