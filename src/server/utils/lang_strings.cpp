#include <std_include.hpp>

#include "lang_strings.hpp"
#include "resources.hpp"

#include <version.h>

namespace utils::lang
{
	std::unordered_map<std::string, std::function<std::string(const std::uint8_t)>> message_vars;

	std::unordered_map<std::string, std::uint8_t> language_code_map =
	{
		{"ANY", LANG_ANY},
		{"KR", LANG_KR},
		{"CH", LANG_CH},
		{"JP", LANG_JP},
		{"RU", LANG_RU},
		{"AR", LANG_AR},
		{"GR", LANG_GR},
		{"IT", LANG_IT},
		{"PR", LANG_PR},
		{"SP", LANG_SP},
		{"FR", LANG_FR},
		{"EN", LANG_EN},
	};

	std::unordered_map<std::string, lang_string_t> load_strings()
	{
		std::unordered_map<std::string, lang_string_t> map;

		auto list = resources::load_json(RESOURCE_LANG_STRINGS);
		for (auto& [string_name, object] : list.items())
		{
			lang_string_t string;
			for (auto& [language, string_value] : object.items())
			{
				const auto lang_code = language_code_map.find(language);
				if (lang_code == language_code_map.end())
				{
					continue;
				}

				string.value[lang_code->second] = string_value.get<std::string>();
			}

			map.insert(std::make_pair(string_name, string));
		}

		return map;
	}

	std::uint8_t get_lang(const std::string& lang_name)
	{
		const auto iter = language_code_map.find(lang_name);
		if (iter != language_code_map.end())
		{
			return iter->second;
		}

		return LANG_ANY;
	}

	const std::string& get_string(const std::string& key, const std::uint8_t language)
	{
		if (key.size() > 64)
		{
			return key;
		}

		const auto string = get_strings().find(key);
		if (string == get_strings().end())
		{
			return key;
		}

		auto lang_idx = language;
		if (lang_idx >= LANG_COUNT)
		{
			lang_idx = LANG_ANY;
		}

		const auto& value = string->second.value[lang_idx];
		const auto& text = value.empty()
			? string->second.value[LANG_ANY]
			: value;

		return text;
	}

	const std::string& get_string(const std::string& key, const std::string& language)
	{
		const auto lang_code = get_lang(language);
		return get_string(key, lang_code);
	}

	const std::unordered_map<std::string, lang_string_t>& get_strings()
	{
		static const auto map = load_strings();
		return map;
	}

	std::string format_vars(const std::string& lang_text, const std::uint8_t lang)
	{
		std::string formatted;
		std::string current_key;

		auto in_key = false;
		auto in_ignore = false;
		for (auto i = 0ull; i < lang_text.size(); i++)
		{
			if (lang_text[i] == '{')
			{
				if (!in_key)
				{
					in_key = true;
					continue;
				}
				else if (!in_ignore)
				{
					in_ignore = true;
				}
			}
			else if (lang_text[i] == '}')
			{
				if (in_key)
				{
					if (!current_key.empty())
					{
						const auto iter = message_vars.find(current_key);
						if (iter != message_vars.end())
						{
							formatted.append(iter->second(lang));
						}
						current_key.clear();
					}

					in_key = false;
					continue;
				}
				else if (in_ignore)
				{
					in_ignore = false;
				}
			}

			if (in_key && !in_ignore)
			{
				current_key.push_back(lang_text[i]);
			}
			else
			{
				formatted.push_back(lang_text[i]);
			}
		}

		if (!current_key.empty())
		{
			formatted.append(current_key);
			current_key.clear();
		}

		return formatted;
	}

	void initialize()
	{
		get_strings();

		register_var("version", [](const std::uint8_t)
		{
			return VERSION;
		});
	}
}
