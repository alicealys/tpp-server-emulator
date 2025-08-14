#include <std_include.hpp>

#include "cmd_get_mgo_purchased_item.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_purchased_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["purchasable_item_list"]["purchasable_item_list"] = nlohmann::json::array();

		return result;
	}
}
