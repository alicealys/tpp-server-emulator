#include <std_include.hpp>

#include "cmd_get_fob_param.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_param::execute(nlohmann::json& data, const std::string& session_key)
	{
		static const auto list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_FOB_PARAM));

		nlohmann::json result;
		result["result"] = "NOERR";
		result["params"] = list;

		return result;
	}
}
