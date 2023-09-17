#include <std_include.hpp>

#include "cmd_get_resource_param.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_resource_param::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["nuclear_develop_costs"][0] = 750000;
		result["nuclear_develop_costs"][1] = 50000;
		result["nuclear_develop_costs"][2] = 75000;

		return result;
	}
}
