#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	void engine::setup_wormhole()
	{
		this->state_["database"]["wormholes"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["wormholes"]["getflagid"] = database::wormholes::get_flag_id;
		this->state_["database"]["wormholes"]["addwormhole"] = database::wormholes::add_wormhole;
		//this->state_["database"]["wormholes"]["findactivewormholes"] = database::wormholes::find_active_wormholes;
		this->state_["database"]["wormholes"]["getwormholestatus"] = database::wormholes::get_wormhole_status;
	}
}
