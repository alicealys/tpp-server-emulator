#include <std_include.hpp>

#include "cmd_send_ipandport.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_send_ipandport::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";
		return result;
	}
}