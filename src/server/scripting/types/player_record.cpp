#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	void engine::setup_player_record()
	{
		this->state_["database"]["playerrecords"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["playerrecords"]["find"] = database::player_records::find;
		this->state_["database"]["playerrecords"]["findorcreate"] = database::player_records::find_or_create;
		this->state_["database"]["playerrecords"]["addsneakresult"] = database::player_records::add_sneak_result;
		this->state_["database"]["playerrecords"]["syncprevvalues"] = database::player_records::sync_prev_values;
		this->state_["database"]["playerrecords"]["findplayersofgrade"] = database::player_records::find_players_of_grade;
		this->state_["database"]["playerrecords"]["findsamegradeplayers"] = database::player_records::find_same_grade_players;
		this->state_["database"]["playerrecords"]["findhighergradeplayers"] = database::player_records::find_higher_grade_players;
		this->state_["database"]["playerrecords"]["setshielddate"] = database::player_records::set_shield_date;
		this->state_["database"]["playerrecords"]["clearshielddate"] = database::player_records::clear_shield_date;
	}
}
