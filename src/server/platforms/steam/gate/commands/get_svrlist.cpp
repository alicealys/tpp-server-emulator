#include <std_include.hpp>

#include "get_svrlist.hpp"

#define HOSTNAME "http://localhost:80/"

namespace tpp
{
	nlohmann::json get_svrlist_handler::execute(const nlohmann::json& data)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["server_num"] = 0;
		result["svrlist"] = nlohmann::json::array();
		result["xuid"] = {};

		return result;
	}
}