#include <std_include.hpp>

#include "cmd_gdpr_check.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	namespace
	{
		const auto gdpr_list = nlohmann::json::parse(utils::nt::load_resource(CMD_GDPR_CHECK));
	}

	nlohmann::json cmd_gdpr_check::execute(const nlohmann::json& data)
	{
		nlohmann::json result = gdpr_list;

		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}