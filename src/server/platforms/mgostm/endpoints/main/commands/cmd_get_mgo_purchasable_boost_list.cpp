#include <std_include.hpp>

#include "cmd_get_mgo_purchasable_boost_list.hpp"

#include "database/models/mgo_item_purchases.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_purchasable_boost_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& list = database::mgo_item_purchases::get_purchaseable_boosts();

		for (auto i = 0u; i < list.size(); i++)
		{
			auto& entry = result["purchasable_boost_list"][i];
			entry["boost_mag"] = list[i].boost_mag;
			entry["boost_type"] = list[i].boost_type;
			entry["effect_seconds"] = list[i].effect_seconds;
			entry["price"] = list[i].price;
			entry["purchase_id"] = list[i].purchase_id;
			entry["purchase_type"] = list[i].purchase_type;
		}

		return result;
	}
}
