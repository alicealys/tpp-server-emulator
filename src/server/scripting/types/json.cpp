#include <std_include.hpp>

#include "../engine.hpp"

namespace tpp::scripting
{
	namespace
	{
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
					return static_cast<int>(val_int);
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
	}

	void engine::setup_json()
	{
		auto json_type = this->state_.new_usertype<nlohmann::json>("json", sol::constructors<nlohmann::json()>());

		json_type[sol::meta_function::index] = sol::overload(
			[](nlohmann::json& value, const sol::this_state s, const std::string& key)
				-> sol::lua_value
			{
				auto& v = value[key];
				const auto type = v.type();
				switch (type)
				{
				case nlohmann::json::value_t::number_integer:
					return {s, v.get<std::int32_t>()};
				case nlohmann::json::value_t::number_float:
					return {s, v.get<float>()};
				case nlohmann::json::value_t::number_unsigned:
					return {s, v.get<std::uint32_t>()};
				case nlohmann::json::value_t::string:
					return {s, v.get<std::string>()};
				case nlohmann::json::value_t::boolean:
					return {s, v.get<bool>()};
				}

				return {s, &v};
			},
			[](nlohmann::json& value, const sol::this_state s, const int index)
				-> sol::lua_value
			{
				auto& v = value[index];
				const auto type = v.type();
				switch (type)
				{
				case nlohmann::json::value_t::number_integer:
					return {s, v.get<std::int32_t>()};
				case nlohmann::json::value_t::number_float:
					return {s, v.get<float>()};
				case nlohmann::json::value_t::number_unsigned:
					return {s, v.get<std::uint32_t>()};
				case nlohmann::json::value_t::string:
					return {s, v.get<std::string>()};
				case nlohmann::json::value_t::boolean:
					return {s, v.get<bool>()};
				}

				return {s, &v};
			}
		);

		json_type[sol::meta_function::new_index] = sol::overload(
			[](nlohmann::json& value, const std::string& key, const sol::lua_value& new_value)
			{
				if (new_value.is<int>())
				{
					value[key] = new_value.as<int>();
					return;
				}

				if (new_value.is<std::string>())
				{
					value[key] = new_value.as<std::string>();
					return;
				}

				if (new_value.is<float>())
				{
					value[key] = new_value.as<float>();
					return;
				}

				if (new_value.is<nlohmann::json>())
				{
					value[key] = new_value.as<nlohmann::json>();
					return;
				}
			},
			[](nlohmann::json& value, const int index, const sol::lua_value& new_value)
			{
				if (new_value.is<int>())
				{
					value[index] = new_value.as<int>();
					return;
				}

				if (new_value.is<std::string>())
				{
					value[index] = new_value.as<std::string>();
					return;
				}

				if (new_value.is<float>())
				{
					value[index] = new_value.as<float>();
					return;
				}

				if (new_value.is<nlohmann::json>())
				{
					value[index] = new_value.as<nlohmann::json>();
					return;
				}
			}
		);

		json_type["dump"] = sol::overload(
			[](nlohmann::json& value)
			{
				return value.dump();
			},
			[](nlohmann::json& value, int indent)
			{
				return value.dump(indent);
			}
		);

		json_type["array"] = []
		{
			return nlohmann::json::array();
		};

		json_type["object"] = []
		{
			return nlohmann::json::object();
		};

		json_type["parse"] = [](const std::string& text)
		{
			return nlohmann::json::parse(text);
		};

		json_type["convert"] = lua_to_json;

		json_type[sol::meta_function::to_string] = [](nlohmann::json& value)
		{
			return std::format("sol.nlohmann::json: {:016X}", reinterpret_cast<size_t>(&value));
		};

#define REGISTER_METHOD(__method__) \
		{ \
			static const auto name = utils::string::replace(#__method__, "_", ""); \
			json_type[name] = [](nlohmann::json& value) \
			{ \
				return value.__method__(); \
			}; \
		} \

		REGISTER_METHOD(is_primitive);
		REGISTER_METHOD(is_structured);
		REGISTER_METHOD(is_null);
		REGISTER_METHOD(is_boolean);
		REGISTER_METHOD(is_number);
		REGISTER_METHOD(is_number_integer);
		REGISTER_METHOD(is_number_unsigned);
		REGISTER_METHOD(is_number_float);
		REGISTER_METHOD(is_object);
		REGISTER_METHOD(is_array);
		REGISTER_METHOD(is_string);
		REGISTER_METHOD(is_binary);
		REGISTER_METHOD(is_discarded);

		json_type["get"] = [](nlohmann::json& value, const sol::this_state s)
			-> sol::lua_value
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
			case nlohmann::json::value_t::object:
				return {s, value};
			default:
				return {s, sol::nil};
			}
		};
	}
}
