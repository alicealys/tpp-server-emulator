#include <std_include.hpp>

#include "cmd_gdpr_check.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_gdpr_check::execute(const nlohmann::json& data)
	{
		static const auto gdpr_list = nlohmann::json::parse(utils::nt::load_resource(CMD_GDPR_CHECK));

		nlohmann::json result = gdpr_list;

		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}
