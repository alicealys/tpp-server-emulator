#include <std_include.hpp>

#include "resources.hpp"
#include "component/console.hpp"

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
			{RESOURCE_TPP_INFORMATIONLIST2, "resources/data/tpp_informationlist2.json"},
			{RESOURCE_ITEM_LIST, "resources/data/item_list.json"},
			{RESOURCE_MINING_RESOURCE, "resources/data/mining_resource.json"},
			{RESOURCE_AREA_LIST, "resources/data/area_list.json"},
			{RESOURCE_FOB_PARAM, "resources/data/fob_param.json"},
			{RESOURCE_SECURITY_SETTINGS_PARAM, "resources/data/security_setting_param.json"},
			{RESOURCE_FOB_DEPLOY_LIST, "resources/data/fob_deploy_list.json"},
			{RESOURCE_STEAM_SHOP_ITEM_LIST, "resources/data/steam_shop_item_list.json"},
			{RESOURCE_MGO_PARAMETERS, "resources/data/mgo_parameters.json"},
			{RESOURCE_MGO_TITLE_LIST, "resources/data/mgo_title_list.json"},
			{RESOURCE_MGO_INFORMATIONLIST2, "resources/data/mgo_informationlist2.json"},
			{RESOURCE_MGO_PURCHASABLE_LIST, "resources/data/mgo_purchasable_list.json"},
			{RESOURCE_MGO_BASE_LOADOUTS, "resources/data/mgo_base_loadouts.json"},
			{RESOURCE_MGO_BASE_AVATAR, "resources/data/mgo_base_avatar.json"},
			{RESOURCE_MGO_GEAR_COLORS, "resources/data/mgo_gear_colors.json"},
			{RESOURCE_MGO_GEAR_LIST, "resources/data/mgo_gear_list.json"},
			{RESOURCE_MGO_WEAPON_COLORS, "resources/data/mgo_weapon_colors.json"},
			{RESOURCE_FOB_EVENT_LIST, "resources/data/fob_event_list.json"},
			{RESOURCE_SHOP_ITEM_LIST, "resources/data/shop_item_list.json"},
			{RESOURCE_COMBAT_DEPLOY_LIST, "resources/data/combat_deploy_list.json"},
			{RESOURCE_DAILY_REWARDS, "resources/data/daily_rewards.json"},
			{RESOURCE_PAY_ITEM_LIST, "resources/data/pay_item_list.json"},
			{RESOURCE_EMBLEM_LIST, "resources/data/emblem_list.json"},
			{RESOURCE_SECURITY_PRODUCT_LIST, "resources/data/security_product_list.json"},
			{RESOURCE_PF_POINTS_TABLE, "resources/data/pf_points_table.json"},

			{RESOURCE_SQL_MYSQL, "resources/sql/mysql.sql"},
			{RESOURCE_SQL_SQLITE3, "resources/sql/sqlite3.sql"},
		};
	}

	std::string load(const std::int32_t resource_id)
	{
		const auto resource_iter = resource_map.find(resource_id);
		if (resource_iter == resource_map.end())
		{
			throw std::runtime_error(std::format("invalid resource id {}", resource_id));
		}

		{
			std::string data;
			if (utils::io::read_file(resource_iter->second, &data))
			{
				return data;
			}
		}

#ifdef _WIN32
		return utils::nt::load_resource(resource_id);
#else
		console::error("resource \"%s\" (%i) not found\n", resource_iter->second.data(), resource_id);
		return {};
#endif
	}

	nlohmann::json load_json(const std::int32_t resource_id)
	{
		return nlohmann::json::parse(load(resource_id));
	}
}
