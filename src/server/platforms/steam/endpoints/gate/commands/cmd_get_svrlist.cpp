#include <std_include.hpp>

#include "cmd_get_svrlist.hpp"

namespace tpp
{
	nlohmann::json cmd_get_svrlist::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["server_num"] = 0;
		result["svrlist"] = nlohmann::json::array();
		result["xuid"] = {};

		return result;
	}
}