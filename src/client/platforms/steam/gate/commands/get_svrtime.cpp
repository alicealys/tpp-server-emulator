#include <std_include.hpp>

#include "get_svrtime.hpp"

#define HOSTNAME "http://localhost:80/"

namespace tpp
{
	nlohmann::json get_svrtime_handler::execute(const nlohmann::json& data)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["date"] = std::time(nullptr);

		return result;
	}
}