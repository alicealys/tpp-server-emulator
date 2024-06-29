#pragma once

#include "component/console.hpp"

#include <utils/string.hpp>

namespace config
{
	using field_type = nlohmann::json::value_t ;
	using field_value = nlohmann::json;
	using validate_callback_t = std::function<bool(const field_value&)>;

	template <typename T>
	using validate_callback_val_t = std::function<bool(const T&)>;

	nlohmann::json read_config();
	void write_config(const nlohmann::json& json);

	nlohmann::json validate_config_field(const std::string& key, const field_value& value);
	std::optional<nlohmann::json> get_default_value(const std::string& key);

	std::optional<nlohmann::json> get_value(const std::string& key);

	template <typename T>
	T get(const std::string& key)
	{
		const auto value_opt = get_value(key);
		if (!value_opt.has_value())
		{
			const auto default_value = get_default_value(key);
			if (default_value.has_value())
			{
				return default_value.value();
			}

			throw std::runtime_error("config field default value not defined");
		}

		const auto validated = validate_config_field(key, *value_opt);
		return validated.get<T>();
	}

	template <typename T>
	T get_or(const std::string& key, const T& default_value, const std::optional<validate_callback_val_t<T>>& validate_cb = {})
	{
		const auto value_opt = get_value(key);
		if (!value_opt.has_value())
		{
			return default_value;
		}

		try
		{
			const auto value = value_opt->get<T>();
			if (validate_cb.has_value() && !validate_cb->operator()(value))
			{
				return default_value;
			}

			return value;
		}
		catch (const std::exception& e)
		{
			console::error("Invalid value for \"%s\" in config: %s\n", key.data(), e.what());
			return default_value;
		}
	}

	nlohmann::json get_raw(const std::string& key);

	template <typename T>
	void set(const std::string& key, const T& value)
	{
		auto cfg = read_config();
		cfg[key] = validate_config_field(key, value);
		write_config(cfg);
	}
}
