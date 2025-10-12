#include <std_include.hpp>

#include "database/models/mgo_item_purchase.hpp"

#include "cmd_get_mgo_purchased_item.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_purchased_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto purchased_items = database::mgo_item_purchase::get_purchase_list(player->get_id());

		for (auto i = 0ull; i < purchased_items.size(); i++)
		{
			const auto info = database::mgo_item_purchase::get_purchase_info(purchased_items[i]);
			if (!info.has_value())
			{
				continue;
			}

			result["purchasable_item_list"]["purchasable_item_list"][i]["purchase_id"] = info->purchase_id;
			result["purchasable_item_list"]["purchasable_item_list"][i]["category"] = info->category;
			result["purchasable_item_list"]["purchasable_item_list"][i]["price"] = info->price;
			result["purchasable_item_list"]["purchasable_item_list"][i]["purchase_type"] = info->purchase_type;
		}

		return result;
	}
}
