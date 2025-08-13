#include <std_include.hpp>

#include "../engine.hpp"

namespace emulator::scripting
{
	void engine::setup_player_data()
	{
		auto player_data_type = this->state_.new_usertype<database::player_data::player_data>("database::player_data::player_data");

#define REGISTER_METHOD(__method__) \
		{ \
			static const auto name = utils::string::replace(#__method__, "_", ""); \
			player_data_type[name] = [](const database::player_data::player_data& data) \
			{ \
				return data.__method__(); \
			}; \
		} \

		//REGISTER_METHOD(get_resource_value);
		//REGISTER_METHOD(copy_resources);
		REGISTER_METHOD(get_player_id);
		//REGISTER_METHOD(get_staff);
		//REGISTER_METHOD(get_unit_level);
		//REGISTER_METHOD(get_unit_count);
		REGISTER_METHOD(get_staff_count);
		REGISTER_METHOD(get_usable_staff_count);
		REGISTER_METHOD(get_motherbase);
		REGISTER_METHOD(get_loadout);
		REGISTER_METHOD(get_emblem);
		REGISTER_METHOD(get_server_gmp);
		REGISTER_METHOD(get_local_gmp);
		REGISTER_METHOD(get_mb_coin);
		REGISTER_METHOD(get_version);
		REGISTER_METHOD(get_last_sync);
		REGISTER_METHOD(get_nuke_count);
		REGISTER_METHOD(get_fob_deploy_damage_param);

		this->state_["database"]["playerdata"] = sol::state::create_table(this->state_.lua_state());

		//this->state_["database"]["playerdata"]["unitnamefromdesignation"] = database::player_data::unit_name_from_designation;
		//this->state_["database"]["playerdata"]["decodebuffer"] = static_cast<std::string(*)(const std::string&)>(database::player_data::decode_buffer);
		//this->state_["database"]["playerdata"]["getmaxresourcevalue"] = database::player_data::get_max_resource_value;
		//this->state_["database"]["playerdata"]["capresourcevalue"] = database::player_data::cap_resource_value;
		//this->state_["database"]["playerdata"]["getlocalresourceratio"] = database::player_data::get_local_resource_ratio;
		//this->state_["database"]["playerdata"]["applydeploydamageparams"] = database::player_data::apply_deploy_damage_params;
		this->state_["database"]["playerdata"]["create"] = database::player_data::create;
		this->state_["database"]["playerdata"]["find"] = database::player_data::find;
		this->state_["database"]["playerdata"]["findorcreate"] = database::player_data::find_or_create;
		this->state_["database"]["playerdata"]["setsoldierbin"] = database::player_data::set_soldier_bin;
		this->state_["database"]["playerdata"]["setsoldierdata"] = database::player_data::set_soldier_data;
		this->state_["database"]["playerdata"]["setsoldierdiff"] = database::player_data::set_soldier_diff;
		this->state_["database"]["playerdata"]["setresources"] = database::player_data::set_resources;
		this->state_["database"]["playerdata"]["setresourcesassync"] = database::player_data::set_resources_as_sync;
		this->state_["database"]["playerdata"]["syncmotherbase"] = database::player_data::sync_motherbase;
		this->state_["database"]["playerdata"]["syncloadout"] = database::player_data::sync_loadout;
		this->state_["database"]["playerdata"]["syncemblem"] = database::player_data::sync_emblem;
		this->state_["database"]["playerdata"]["spendcoins"] = database::player_data::spend_coins;
		this->state_["database"]["playerdata"]["addcoins"] = database::player_data::add_coins;
		this->state_["database"]["playerdata"]["getnukecount"] = database::player_data::get_nuke_count;
		this->state_["database"]["playerdata"]["getplayernukecount"] = database::player_data::get_player_nuke_count;
		this->state_["database"]["playerdata"]["setfobdeploydamageparam"] = database::player_data::set_fob_deploy_damage_param;
		this->state_["database"]["playerdata"]["findwithnukes"] = database::player_data::find_with_nukes;
	}
}
