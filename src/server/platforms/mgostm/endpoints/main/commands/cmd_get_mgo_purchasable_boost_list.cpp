#include <std_include.hpp>

#include "cmd_get_mgo_purchasable_boost_list.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_purchasable_boost_list::cmd_get_mgo_purchasable_boost_list()
	{
		auto list = utils::resources::load_json(RESOURCE_MGO_PURCHASABLE_LIST);
		this->list_["purchasable_boost_list"] = list["purchasable_boost_list"];
	}

	nlohmann::json cmd_get_mgo_purchasable_boost_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
