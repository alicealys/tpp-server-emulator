#include <std_include.hpp>

#include "cmd_get_mgo_title_list.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_title_list::cmd_get_mgo_title_list()
	{
		this->list_["title_list"] = utils::resources::load_json(RESOURCE_MGO_TITLE_LIST);
	}

	nlohmann::json cmd_get_mgo_title_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
