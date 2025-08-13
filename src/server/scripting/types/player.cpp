#include <std_include.hpp>

#include "../engine.hpp"

namespace emulator::scripting
{
	void engine::setup_player()
	{
		auto player_type = this->state_.new_usertype<database::players::player>("database::players::player");

#define REGISTER_METHOD(__method__) \
		{ \
			static const auto name = utils::string::replace(#__method__, "_", ""); \
			player_type[name] = [](const database::players::player& player) \
			{ \
				return player.__method__(); \
			}; \
		} \

		REGISTER_METHOD(get_id);
		REGISTER_METHOD(get_account_id);
		REGISTER_METHOD(get_login_password);
		REGISTER_METHOD(get_crypto_key);
		REGISTER_METHOD(get_smart_device_id);
		REGISTER_METHOD(get_currency);
		REGISTER_METHOD(get_ex_ip);
		REGISTER_METHOD(get_in_ip);
		REGISTER_METHOD(get_ex_port);
		REGISTER_METHOD(get_in_port);
		REGISTER_METHOD(get_nat);
		REGISTER_METHOD(get_last_update);
		REGISTER_METHOD(get_creation_time);
		REGISTER_METHOD(is_security_challenge_enabled);
		REGISTER_METHOD(get_session_id);
		REGISTER_METHOD(get_name);

		this->state_["database"]["players"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["players"]["find"] = database::players::find;
		this->state_["database"]["players"]["exists"] = database::players::exists;
		this->state_["database"]["players"]["findfromaccount"] = database::players::find_from_account;
		this->state_["database"]["players"]["find_bysessionid"] = database::players::find_by_session_id;
		this->state_["database"]["players"]["findorinsert"] = database::players::find_or_insert;
		this->state_["database"]["players"]["generateloginpassword"] = database::players::generate_login_password;
		this->state_["database"]["players"]["generatesessionid"] = database::players::generate_session_id;
		this->state_["database"]["players"]["generatecryptokey"] = database::players::generate_crypto_key;
		this->state_["database"]["players"]["updatesession"] = database::players::update_session;
		this->state_["database"]["players"]["setipandport"] = database::players::set_ip_and_port;
		this->state_["database"]["players"]["getplayerlist"] = database::players::get_player_list;
		this->state_["database"]["players"]["abortmotherbase"] = database::players::abort_mother_base;
		this->state_["database"]["players"]["setactivesneak"] = database::players::set_active_sneak;

		this->state_["database"]["players"]["findactivesneak"] = sol::overload(
			static_cast<std::optional<database::players::sneak_info>(*)(const std::uint64_t, bool, bool)>(database::players::find_active_sneak),
			static_cast<std::optional<database::players::sneak_info>(*)(const std::uint64_t, const uint32_t, const std::uint32_t, bool, bool)>
				(database::players::find_active_sneak)
		);

		this->state_["database"]["players"]["findactivesneakfromplayer"] = database::players::find_active_sneak_from_player;
		this->state_["database"]["players"]["getactivesneak"] = database::players::get_active_sneak;
		this->state_["database"]["players"]["setsecuritychallenge"] = database::players::set_security_challenge;
		this->state_["database"]["players"]["findwithsecuritychallenge"] = database::players::find_with_security_challenge;
		this->state_["database"]["players"]["getplayercount"] = database::players::get_player_count;
		this->state_["database"]["players"]["getonlineplayercount"] = sol::overload(
			static_cast<uint64_t(*)()>(database::players::get_online_player_count),
			[](const int milliseconds)
			{
				return database::players::get_online_player_count(std::chrono::milliseconds(milliseconds));
			}
		);
	}
}
