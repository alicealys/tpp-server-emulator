#include <std_include.hpp>

#include "config.hpp"

#include <utils/io.hpp>

namespace config
{
	namespace
	{
		struct field_definition_t
		{
			field_type type;
			field_value default_value;
			std::optional<validate_callback_t> validate_value = {};
		};

		template <typename... Args>
		std::pair<std::string, field_definition_t> define_field(const std::string& name, Args&&... args)
		{
			return std::make_pair(name, field_definition_t{std::forward<Args>(args)...});
		}

		std::unordered_map<std::string, field_definition_t> field_definitions =
		{
			{define_field("base_url", field_type::string, "http://localhost:80")},
			{define_field("https_port", field_type::number_unsigned, 443)},
			{define_field("http_port", field_type::number_unsigned, 80)},
			{define_field("database_user", field_type::string, "root")},
			{define_field("database_password", field_type::string, "root")},
			{define_field("database_host", field_type::string, "localhost")},
			{define_field("database_port", field_type::number_unsigned, 3306)},
			{define_field("database_name", field_type::string, "mgstpp")},
			{define_field("use_konami_auth", field_type::boolean, false)},
			{define_field("cert_file", field_type::string, "")},
			{define_field("key_file", field_type::string, "")},
			{define_field("use_tmp_folder", field_type::boolean, true)},
		};

		std::string get_config_file_path()
		{
			return "config.json";
		}
	}

	std::optional<nlohmann::json> get_value(const std::string& key)
	{
		const auto cfg = read_config();
		if (!cfg.is_object())
		{
			return {};
		}

		const auto nodes = utils::string::split(key, '.');
		auto obj = &cfg;
		for (auto i = 0; i < nodes.size(); i++)
		{
			const auto& node = nodes[i];
			if (!obj->contains(nodes[i]))
			{
				return {};
			}

			obj = &obj->at(node);
		}

		return {*obj};
	}

	nlohmann::json validate_config_field(const std::string& key, const nlohmann::json& value)
	{
		const auto iter = field_definitions.find(key);
		if (iter == field_definitions.end())
		{
			return value;
		}

		if (value.type() != iter->second.type)
		{
			return iter->second.default_value;
		}

		if (iter->second.validate_value.has_value())
		{
			const auto& validate_value = iter->second.validate_value.value();
			if (!validate_value(value))
			{
				iter->second.default_value;
			}
		}

		return value;
	}

	std::optional<nlohmann::json> get_default_value(const std::string& key)
	{
		const auto iter = field_definitions.find(key);
		if (iter == field_definitions.end())
		{
			return {};
		}

		return {iter->second.default_value};
	}

	nlohmann::json get_raw(const std::string& key)
	{
		const auto cfg = read_config();
		if (!cfg.is_object() || !cfg.contains(key))
		{
			const auto default_value = get_default_value(key);
			if (default_value.has_value())
			{
				return default_value.value();
			}

			return {};
		}

		return validate_config_field(key, cfg[key]);
	}

	void write_config(const nlohmann::json& json)
	{
		try
		{
			const auto path = get_config_file_path();
			const auto str = json.dump(4);
			utils::io::write_file(path, str, false);
		}
		catch (const std::exception& e)
		{
			printf("Failed to write config file: %s\n", e.what());
		}
	}

	nlohmann::json read_config()
	{
		const auto path = get_config_file_path();
		if (!utils::io::file_exists(path))
		{
			return {};
		}

		try
		{
			const auto data = utils::io::read_file(path);
			return nlohmann::json::parse(data);
		}
		catch (const std::exception& e)
		{
			printf("Failed to parse config file: %s\n", e.what());
			utils::io::write_file(path, "{}", false);
		}

		return {};
	}
}
