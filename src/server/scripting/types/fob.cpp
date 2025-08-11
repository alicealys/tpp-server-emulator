#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	void engine::setup_fob()
	{
		this->state_["database"]["fobs"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["fobs"]["getarealist"] = database::fobs::get_area_list;
		this->state_["database"]["fobs"]["getarea"] = database::fobs::get_area;
		this->state_["database"]["fobs"]["getfoblist"] = database::fobs::get_fob_list;
		this->state_["database"]["fobs"]["create"] = database::fobs::create;
		this->state_["database"]["fobs"]["syncdata"] = database::fobs::sync_data;
		this->state_["database"]["fobs"]["getfob"] = database::fobs::get_fob;
	}
}
