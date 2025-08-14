#include <std_include.hpp>

#include "cmd_get_mgo_purchasable_item_list.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_purchasable_item_list::cmd_get_mgo_purchasable_item_list()
	{
		auto list = utils::resources::load_json(RESOURCE_MGO_PURCHASABLE_LIST);
		this->list_["purchasable_item_list"] = list["purchasable_item_list"];
	}

	nlohmann::json cmd_get_mgo_purchasable_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
