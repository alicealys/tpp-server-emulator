#include <std_include.hpp>

#include "json.hpp"

namespace tpp::scripting
{
	void engine::setup_json()
	{
		auto json_type = this->state_.new_usertype<nlohmann::json>("json", sol::constructors<nlohmann::json()>());

		json_type[sol::meta_function::index] = sol::overload(
			[](nlohmann::json& value, const std::string& key)
				-> nlohmann::json&
			{
				return value[key];
			},
			[](nlohmann::json& value, const int index)
				-> nlohmann::json&
			{
				return value[index];
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

		json_type[sol::meta_function::to_string] = [](nlohmann::json& value)
		{
			return utils::string::va("sol.nlohmann::json: %p", &value);
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

		json_type["get"] = [](nlohmann::json& value)
			-> sol::lua_value
		{
			const auto type = value.type();
			switch (type)
			{
			case nlohmann::json::value_t::boolean:
				return value.get<bool>();
			case nlohmann::json::value_t::number_float:
				return value.get<float>();
			case nlohmann::json::value_t::number_integer:
				return value.get<int>();
			case nlohmann::json::value_t::number_unsigned:
				return value.get<unsigned int>();
			case nlohmann::json::value_t::string:
				return value.get<std::string>();
			case nlohmann::json::value_t::array:
			case nlohmann::json::value_t::object:
				return value;
			default:
				return {};
			}
		};
	}
}
