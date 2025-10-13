#include <std_include.hpp>

#include "database/models/shop_purchases.hpp"

#include "cmd_get_shop_item_name_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_shop_item_name_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& list = database::shop_purchases::get_shop_item_list();
		for (auto i = 0ull; i < list.size(); i++)
		{
			result["item"][i]["id"] = list[i].item_type;
			result["item"][i]["name"] = list[i].name;
			result["item"][i]["price"] = list[i].price;
		}

		return result;
	}
}
