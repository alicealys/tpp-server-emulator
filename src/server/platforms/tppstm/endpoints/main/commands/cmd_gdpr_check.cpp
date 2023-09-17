#include <std_include.hpp>

#include "cmd_gdpr_check.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_gdpr_check::execute(nlohmann::json& data, const std::string& session_key)
	{
		static const auto gdpr_list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_GDPR_CHECK));

		nlohmann::json result = gdpr_list;

		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}
