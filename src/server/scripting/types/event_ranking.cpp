#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	void engine::setup_event_ranking()
	{
		this->state_["database"]["eventrankings"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["eventrankings"]["getlookuptypefromname"] = database::event_rankings::get_lookup_type_from_name;
		this->state_["database"]["eventrankings"]["geteventtypefromid"] = database::event_rankings::get_event_type_from_id;
		this->state_["database"]["eventrankings"]["createentries"] = database::event_rankings::create_entries;
		this->state_["database"]["eventrankings"]["seteventvalue"] = database::event_rankings::set_event_value;
		this->state_["database"]["eventrankings"]["incrementeventvalue"] = database::event_rankings::increment_event_value;
		this->state_["database"]["eventrankings"]["setvalueifbigger"] = database::event_rankings::set_value_if_bigger;
		this->state_["database"]["eventrankings"]["getplayerrank"] = database::event_rankings::get_player_rank;
		this->state_["database"]["eventrankings"]["getentries"] = database::event_rankings::get_entries;
		this->state_["database"]["eventrankings"]["getlastupdate"] = database::event_rankings::get_last_update;
	}
}
