#include <std_include.hpp>

#include "cmd_get_purchasable_area_list.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_purchasable_area_list::execute(nlohmann::json& data, const std::string& session_key)
	{
		static const auto list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_AREA_LIST));

		nlohmann::json result;
		result["result"] = "NOERR";
		result["area"] = list;

		return result;
	}
}
