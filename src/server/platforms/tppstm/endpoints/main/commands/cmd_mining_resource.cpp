#include <std_include.hpp>

#include "cmd_mining_resource.hpp"

#include "database/models/items.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_mining_resource::execute(nlohmann::json& data, const std::string& session_key)
	{
		static const auto list = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_MINING_RESOURCE));

		nlohmann::json result = list;
		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}
