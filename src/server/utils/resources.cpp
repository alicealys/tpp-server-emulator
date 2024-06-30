#include <std_include.hpp>

#include "resources.hpp"

#include <utils/nt.hpp>
#include <utils/io.hpp>

namespace utils::resources
{
	namespace
	{
		std::unordered_map<std::int32_t, std::string> resource_map =
		{
			{RESOURCE_GDPR_CHECK, "resources/data/gdpr_check.json"},
			{RESOURCE_CHALLENGE_TASK_REWARDS, "resources/data/challenge_task_rewards.json"},
			{RESOURCE_LOGIN_PARAM, "resources/data/login_param.json"},
			{RESOURCE_INFORMATIONLIST2, "resources/data/informationlist2.json"},
			{RESOURCE_ITEM_LIST, "resources/data/item_list.json"},
			{RESOURCE_MINING_RESOURCE, "resources/data/mining_resource.json"},
			{RESOURCE_AREA_LIST, "resources/data/area_list.json"},
			{RESOURCE_FOB_PARAM, "resources/data/fob_param.json"},
			{RESOURCE_SECURITY_SETTINGS_PARAM, "resources/data/security_setting_param.json"},
			{RESOURCE_FOB_DEPLOY_LIST, "resources/data/fob_deploy_list.json"},
			{RESOURCE_STEAM_SHOP_ITEM_LIST, "resources/data/steam_shop_item_list.json"},
		};
	}

	std::string load(const std::int32_t resource_id)
	{
		const auto resource_iter = resource_map.find(resource_id);
		if (resource_iter == resource_map.end())
		{
			return {};
		}

		{
			std::string data;
			if (utils::io::read_file(resource_iter->second, &data))
			{
				return data;
			}
		}

		return utils::nt::load_resource(resource_id);
	}

	nlohmann::json load_json(const std::int32_t resource_id)
	{
		return nlohmann::json::parse(load(resource_id));
	}
}
