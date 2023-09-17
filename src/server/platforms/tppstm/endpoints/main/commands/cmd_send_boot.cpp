#include <std_include.hpp>

#include "cmd_send_boot.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_send_boot::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";
		return result;
	}
}
