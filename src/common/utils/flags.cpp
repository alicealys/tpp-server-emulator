#include "flags.hpp"
#include "string.hpp"

namespace utils::flags
{
	namespace
	{
		using flag_map_t = std::unordered_map<std::string, std::optional<std::string>>;

		flag_map_t& get_flags()
		{
			static flag_map_t map = {};
			return map;
		}

		void parse_flags(flag_map_t& flags, int argc, char** argv)
		{
			flags.clear();

			if (argv == nullptr)
			{
				return;
			}

			std::optional<std::string> last_flag{};
			for (auto i = 0; i < argc; ++i)
			{
				std::string flag_str(argv[i]);
				if (flag_str[0] == '-')
				{
					flag_str.erase(flag_str.begin());
					last_flag = flag_str;
					flags[flag_str] = {};
				}
				else if (last_flag.has_value())
				{
					const auto& flag = last_flag.value();
					flags[flag] = flag_str;
					last_flag = {};
				}
			}
		}
	}

	void init(int argc, char** argv)
	{
		parse_flags(get_flags(), argc, argv);
	}

	bool has_flag(const std::string& flag)
	{
		for (const auto& [name, value] : get_flags())
		{
			if (string::to_lower(name) == string::to_lower(flag))
			{
				return true;
			}
		}

		return false;
	}

	std::optional<std::string> get_flag(const std::string& flag)
	{
		for (const auto& [name, value] : get_flags())
		{
			if (string::to_lower(name) == string::to_lower(flag))
			{
				return value;
			}
		}

		return {};
	}

	std::optional<std::string> get_flag(const std::string& flag, const std::string& shortname)
	{
		auto value = get_flag(flag);

		if (!value.has_value())
		{
			value = get_flag(shortname);
		}

		return value;
	}

	std::string get_flag(const std::string& flag, const std::string& shortname,
		const std::string& default_)
	{
		const auto value = get_flag(flag, shortname);

		if (!value.has_value())
		{
			return default_;
		}

		return value.value();
	}
}
