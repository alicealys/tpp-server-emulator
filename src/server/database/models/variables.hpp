#pragma once

#include "../database.hpp"

namespace database::variables
{
	class variable
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(variable_name, sqlpp::text);
		DEFINE_FIELD(variable_value, sqlpp::text);
		DEFINE_TABLE(variables, id_field_t, variable_name_field_t, variable_value_field_t);

		inline static table_t table;

	};

	void set(const std::string& name, const nlohmann::json& value);
	std::optional<nlohmann::json> get(const std::string& name);

	template <typename T>
	T get(const std::string& name, const T& default_value)
	{
		const auto value = get(name);
		if (value.has_value())
		{
			return value->get<T>();
		}

		return value.has_value() ? value->get<T>() : default_value;
	}

	void access_with_lock(const std::string& name, const std::function<void()>& callback);
	void register_lock(const std::string& name);
}
