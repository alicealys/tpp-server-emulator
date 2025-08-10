#include <std_include.hpp>

#include "database/database.hpp"
#include "database/vars.hpp"

#include "database.hpp"

namespace tpp::scripting
{
	void engine::setup_database()
	{
		this->state_["database"] = sol::state::create_table(this->state_.lua_state());
		this->state_["database"]["vars"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["vars"]["session_heartbeat"] = database::vars.session_heartbeat.count();
		this->state_["database"]["vars"]["session_timeout"] = database::vars.session_timeout.count();
		this->state_["database"]["vars"]["nuclear_find_probability"] = database::vars.nuclear_find_probability;
		this->state_["database"]["vars"]["wormhole_duration"] = database::vars.wormhole_duration.count();
		this->state_["database"]["vars"]["max_server_gmp"] = database::vars.max_server_gmp;
		this->state_["database"]["vars"]["max_local_gmp"] = database::vars.max_local_gmp;
		this->state_["database"]["vars"]["gmp_ratio"] = database::vars.gmp_ratio;
		this->state_["database"]["vars"]["item_dev_limit"] = database::vars.item_dev_limit;
		this->state_["database"]["vars"]["unlock_all_items"] = database::vars.unlock_all_items;
		this->state_["database"]["vars"]["cost_factor_generic"] = database::vars.cost_factor_generic;
		this->state_["database"]["vars"]["cost_factor_item_dev"] = database::vars.cost_factor_item_dev;
		this->state_["database"]["vars"]["cost_factor_platform_construction"] = database::vars.cost_factor_platform_construction;
	}
}
