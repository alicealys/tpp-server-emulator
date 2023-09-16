#include <std_include.hpp>

#include "cmd_get_abolition_count.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_abolition_count::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";
		result["count"] = 3;
		result["date"] = std::time(nullptr);
		result["max"] = std::numeric_limits<int>::max();
		result["num"] = 1000;
		result["status"] = 0;

		return result;
	}
}
