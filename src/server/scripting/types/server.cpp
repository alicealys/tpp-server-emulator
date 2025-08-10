#include <std_include.hpp>

#include "server.hpp"

namespace tpp::scripting
{
	void engine::setup_server()
	{
		this->state_["server"] = sol::state::create_table(this->state_.lua_state());

		//this->state_["server"]["getresource"] = utils::resources::load

		this->state_["server"]["registercommand"] = [&](const std::string& name, const sol::protected_function& handler)
		{
			console::log("Registering script command \"%s\"", name.data());
			this->command_handlers_.insert(std::make_pair(name, handler));
		};
	}
}
