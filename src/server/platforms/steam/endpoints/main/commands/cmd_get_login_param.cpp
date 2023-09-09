#include <std_include.hpp>

#include "cmd_get_login_param.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_login_param::execute(const nlohmann::json& data, const std::string& session_key)
	{
		static const auto list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_LOGIN_PARAM));

		nlohmann::json result = list;

		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}
