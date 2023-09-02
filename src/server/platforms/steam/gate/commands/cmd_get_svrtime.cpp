#include <std_include.hpp>

#include "cmd_get_svrtime.hpp"

#define HOSTNAME "http://localhost:80/"

namespace tpp
{
	nlohmann::json cmd_get_svrtime::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["date"] = std::time(nullptr);

		return result;
	}
}