#include <std_include.hpp>

#include "cmd_get_security_info.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_security_info::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["end_date"] = 0;
		result["in_contract"] = 0;
		result["in_interval"] = 0;
		result["interval_end_date"] = 0;

		return result;
	}
}
