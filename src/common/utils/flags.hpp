#pragma once

#include <string>
#include <optional>
#include <unordered_map>

namespace utils::flags
{
	bool has_flag(const std::string& flag);
	std::optional<std::string> get_flag(const std::string& flag);
	std::optional<std::string> get_flag(const std::string& flag, const std::string& shortname);
	std::string get_flag(const std::string& flag, const std::string& shortname,
		const std::string& default_);
}
