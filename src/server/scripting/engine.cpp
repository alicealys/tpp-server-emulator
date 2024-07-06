#include <std_include.hpp>

#include "engine.hpp"

#include <utils/io.hpp>

namespace scripting::engine
{
	namespace
	{
		std::optional<std::string> read_cmd_script(const std::string& command)
		{
			const auto lower = utils::string::to_lower(command);
			const auto path = std::format("scripts\\commands\\{}.lua", lower);

			std::string data;
			if (utils::io::read_file(path, &data))
			{
				return {data};
			}

			return {};
		}

		nlohmann::json lua_to_json(const sol::lua_value& value);

		nlohmann::json table_to_json(const sol::lua_value& value)
		{
			const auto table = value.as<sol::table>();
			const sol::state_view state_view = value.value().lua_state();
			auto to_string = state_view["tostring"];

			nlohmann::json array_;
			nlohmann::json object;

			auto is_array = true;
			auto last_index = 0;

			for (const auto& [k, v] : table)
			{
				const auto is_numeric = k.is<int>();
				if (!is_numeric)
				{
					is_array = false;
					array_.clear();
				}
				else
				{
					const auto index = k.as<int>();
					if (index - last_index != 1)
					{
						is_array = false;
						array_.clear();
					}

					last_index = index;
				}

				const auto value_json = lua_to_json(v);

				if (is_array)
				{
					array_.push_back(value_json);
				}

				const auto key_str = to_string(k).get<std::string>();
				object[key_str] = value_json;
			}

			if (is_array)
			{
				return array_;
			}
			else
			{
				return object;
			}
		}
		
		nlohmann::json lua_to_json(const sol::lua_value& value)
		{
			const auto type = value.value().get_type();

			switch (type)
			{
			case sol::type::boolean:
				return value.as<bool>();
			case sol::type::number:
			{
				const auto val_float = value.as<float>();
				const auto val_int = static_cast<float>(value.as<int>());
				if (val_int == val_float)
				{
					return val_int;
				}

				return val_float;
			}
			case sol::type::string:
				return value.as<std::string>();
			case sol::type::table:
				return table_to_json(value);
			default:
				return {};
			}
		}

		sol::lua_value json_to_lua(sol::state& s, const nlohmann::json& value);

		sol::lua_value json_array_to_lua(sol::state& s, const nlohmann::json& value)
		{
			auto table = sol::table::create(s.lua_state());

			for (auto i = 0; i < value.size(); i++)
			{
				table[i + 1] = json_to_lua(s, value[i]);
			}

			return table;
		}

		sol::lua_value json_object_to_lua(sol::state& s, const nlohmann::json& value)
		{
			auto table = sol::table::create(s.lua_state());

			for (auto& [k, v] : value.items())
			{
				table[k] = json_to_lua(s, v);
			}

			return table;
		}

		sol::lua_value json_to_lua(sol::state& s, const nlohmann::json& value)
		{
			const auto type = value.type();
			switch (type)
			{
			case nlohmann::json::value_t::boolean:
				return {s, value.get<bool>()};
			case nlohmann::json::value_t::number_float:
				return {s, value.get<float>()};
			case nlohmann::json::value_t::number_integer:
				return {s, value.get<int>()};
			case nlohmann::json::value_t::number_unsigned:
				return {s, value.get<unsigned int>()};
			case nlohmann::json::value_t::string:
				return {s, value.get<std::string>()};
			case nlohmann::json::value_t::array:
				return json_array_to_lua(s, value);
			case nlohmann::json::value_t::object:
				return json_object_to_lua(s, value);
			default:
				return sol::lua_value{s, sol::nil};
			}
		}
	}

	void initialize_player_type(sol::state& state)
	{
		auto player_type = state.new_usertype<database::players::player>("database::players::player");

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
	}

	void initialize_state(sol::state& state)
	{
		state["database"] = sol::state::create_table(state.lua_state());
		state["database"]["players"] = sol::state::create_table(state.lua_state());

		state["database"]["players"]["find"] = database::players::find;
		state["database"]["players"]["findfromaccount"] = database::players::find_from_account;
		state["database"]["players"]["findbysessionid"] = database::players::find_by_session_id;

		initialize_player_type(state);

		state.open_libraries
		(
			sol::lib::base,
			sol::lib::package,
			sol::lib::io,
			sol::lib::string,
			sol::lib::os,
			sol::lib::math,
			sol::lib::table
		);
	}

	std::optional<nlohmann::json> execute_command_hook(const std::string& command, nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		static const auto use_lua_scripts = config::get<bool>("use_lua_scripts");
		if (!use_lua_scripts)
		{
			return {};
		}

		const auto data_opt = read_cmd_script(command);
		if (!data_opt.has_value())
		{
			return {};
		}

		sol::state state;
		initialize_state(state);

		const auto script_result = state.safe_script(data_opt.value(), command);
		if (!script_result.valid())
		{
			const sol::error error = script_result;
			console::error("Error executing script \"%s.lua\": %s\n", command.data(), error.what());
			return {};
		}

		state["getdatastring"] = [&]()
		{
			return data.dump(4);
		};

		const auto exec_func = state["execute"].get<sol::protected_function>();
		const auto data_lua = json_to_lua(state, data);

		sol::protected_function_result result;
		if (player.has_value())
		{
			result = exec_func(command, data_lua, player.value());
		}
		else
		{
			result = exec_func(command, data_lua);
		}

		if (!result.valid())
		{
			const sol::error error = result;
			console::error("Error executing command in script \"%s.lua\": %s\n", command.data(), error.what());
			return {};
		}

		const auto result_value = result.get<sol::lua_value>();
		if (result_value.value().get_type() == sol::type::nil)
		{
			return {};
		}

		try
		{
			const auto result_json = lua_to_json(result_value);
			return {result_json};
		}
		catch (const std::exception& e)
		{
			console::error("Error parsing script result \"%s.lua\": %s\n", command.data(), e.what());
			return {};
		}
	}
}
