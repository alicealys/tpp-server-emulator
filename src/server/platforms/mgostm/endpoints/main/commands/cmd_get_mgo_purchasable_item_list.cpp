#include <std_include.hpp>

#include "database/models/mgo_item_purchase.hpp"

#include "cmd_get_mgo_purchasable_item_list.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_purchasable_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& map = database::mgo_item_purchase::get_purchasable_item_map();

		auto index = 0;
		for (const auto& [id, info] : map)
		{
			result["purchasable_item_list"]["purchasable_item_list"][index]["category"] = info.category;
			result["purchasable_item_list"]["purchasable_item_list"][index]["price"] = info.price;
			result["purchasable_item_list"]["purchasable_item_list"][index]["purchase_id"] = info.purchase_id;
			result["purchasable_item_list"]["purchasable_item_list"][index]["purchase_type"] = info.purchase_type;
			index++;
		}

		return result;
	}
}
