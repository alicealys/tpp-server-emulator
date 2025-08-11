#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	void engine::setup_item()
	{
		this->state_["database"]["items"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["items"]["getstaticlistjson"] = database::items::get_static_list_json;
		this->state_["database"]["items"]["getstaticlist"] = database::items::get_static_list;
		//this->state_["database"]["items"]["getstaticmap"] = database::items::get_static_map;
		this->state_["database"]["items"]["getitemdata"] = database::items::get_item_data;
		//this->state_["database"]["items"]["getitemlist"] = database::items::get_item_list;
		this->state_["database"]["items"]["getitem"] = database::items::get_item;
		this->state_["database"]["items"]["create"] = database::items::create;
		this->state_["database"]["items"]["remove"] = database::items::remove;
		this->state_["database"]["items"]["forcedevelop"] = database::items::force_develop;
	}
}
