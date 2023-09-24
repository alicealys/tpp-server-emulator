#include <std_include.hpp>

#include "cmd_calc_cost_fob_deploy_replace.hpp"

#include "database/models/fobs.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_calc_cost_fob_deploy_replace::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["cost"] = 0;
		return result;
	}
}
