#include <std_include.hpp>

#include "cmd_sync_mother_base.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_mother_base::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";
		return result;
	}
}
