#include <std_include.hpp>

#include "database/models/mgo_color_purchase.hpp"

#include "cmd_get_mgo_purchasable_gear.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_purchasable_gear::cmd_get_mgo_purchasable_gear()
	{
		auto list = utils::resources::load_json(RESOURCE_MGO_PURCHASABLE_LIST);
		this->list_ = list["purchasable_gear_list"];
	}

	nlohmann::json cmd_get_mgo_purchasable_gear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!data["gear_id_list"].is_object() || !data["gear_id_list"].contains("gear_id_list") || !data["gear_id_list"]["gear_id_list"].is_array())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& list = data["gear_id_list"]["gear_id_list"];
		result["purchasable_gear_list"]["purchasable_gear_list"] = nlohmann::json::array();
		auto& result_list = result["purchasable_gear_list"]["purchasable_gear_list"];

		const auto purchased_colors = database::mgo_color_purchase::get_all_purchased_colors(player->get_id(), database::mgo_color_purchase::gear);
		const auto has_gear = [&](const std::uint32_t gear_id)
		{
			const auto iter = std::ranges::find_if(purchased_colors.begin(), purchased_colors.end(), 
				[&](const database::mgo_color_purchase::mgo_color_purchase& purchase)
				{
					return purchase.get_item_id() == gear_id;
				}
			);

			return iter != purchased_colors.end();
		};

		for (auto i = 0ull; i < list.size(); i++)
		{
			const auto gear_id = list[i].get<std::uint32_t>();
			const auto purchased = has_gear( gear_id);
			const auto gear_info = database::mgo_color_purchase::get_gear_info(gear_id);

			result_list[i]["already_released"] = 1;
			result_list[i]["already_purchased"] = purchased ? 1 : 0;
			result_list[i]["default_color"] = gear_info.default_color;
			result_list[i]["gear_id"] = gear_id;
			result_list[i]["point"] = gear_info.point;
			result_list[i]["prestige"] = gear_info.prestige;
			result_list[i]["purchase_type"] = gear_info.purchase_type;
		}

		return result;
	}

	bool cmd_get_mgo_purchasable_gear::needs_player()
	{
		return true;
	}
}
