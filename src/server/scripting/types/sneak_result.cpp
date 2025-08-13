#include <std_include.hpp>

#include "../engine.hpp"

namespace emulator::scripting
{
	void engine::setup_sneak_result()
	{
		this->state_["database"]["sneakresults"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["sneakresults"]["addsneakresult"] = database::sneak_results::add_sneak_result;
		this->state_["database"]["sneakresults"]["getsneakresults"] = database::sneak_results::get_sneak_results;
		this->state_["database"]["sneakresults"]["getsneakresult"] = database::sneak_results::get_sneak_result;
	}
}
