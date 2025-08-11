#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	void engine::setup_player_follow()
	{
		this->state_["database"]["playerfollows"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["playerfollows"]["addfollow"] = database::player_follows::add_follow;
		this->state_["database"]["playerfollows"]["removefollow"] = database::player_follows::remove_follow;
		this->state_["database"]["playerfollows"]["getfollows"] = database::player_follows::get_follows;
		this->state_["database"]["playerfollows"]["getfollowers"] = database::player_follows::get_followers;
	}
}
