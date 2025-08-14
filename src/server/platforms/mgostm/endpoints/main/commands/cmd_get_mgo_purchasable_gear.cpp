#include <std_include.hpp>

#include "cmd_get_mgo_purchasable_gear.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_purchasable_gear::cmd_get_mgo_purchasable_gear()
	{
		auto list = utils::resources::load_json(RESOURCE_MGO_PURCHASABLE_LIST);
		this->list_["purchasable_gear_list"] = list["purchasable_gear_list"];
	}

	nlohmann::json cmd_get_mgo_purchasable_gear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
