#include <std_include.hpp>

#include "player.hpp"

namespace tpp::scripting
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
		this->state_["database"]["players"]["findfromaccount"] = database::players::find_from_account;
		this->state_["database"]["players"]["findbysessionid"] = database::players::find_by_session_id;
	}
}
