#pragma once

namespace config
{
	typedef nlohmann::json::value_t field_type;
	typedef nlohmann::json field_value;

	nlohmann::json read_config();
	void write_config(const nlohmann::json& json);

	nlohmann::json validate_config_field(const std::string& key, const field_value& value);
	std::optional<nlohmann::json> get_default_value(const std::string& key);

	template <typename T>
	T get(const std::string& key)
	{
		const auto cfg = read_config();
		if (!cfg.is_object() || !cfg.contains(key))
		{
			const auto default_value = get_default_value(key);
			if (default_value.has_value())
			{
				return default_value.value();
			}

			throw std::runtime_error("config field default value not defined");
		}

		const auto value = validate_config_field(key, cfg[key]);
		return value.get<T>();
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
