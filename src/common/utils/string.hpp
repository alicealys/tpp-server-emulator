#pragma once
#include "memory.hpp"
#include <cstdint>
#include <string>

namespace utils::string
{
	std::vector<std::string> split(const std::string& s, char delim);

	std::string to_lower(std::string text);
	std::string to_upper(std::string text);
	bool starts_with(const std::string& text, const std::string& substring);
	bool ends_with(const std::string& text, const std::string& substring);

	std::string dump_hex(const std::string& data, const std::string& separator = " ", bool upper_case = true);

	void strip(const char* in, char* out, int max);

	std::string convert(const std::wstring& wstr);
	std::wstring convert(const std::string& str);

	std::string replace(std::string str, const std::string& from, const std::string& to);

	bool match_compare(const std::string& input, const std::string& text, const bool exact);

	bool is_numeric(const std::string& text);

	std::string trim(const std::string& str, const std::string& whitespace = " \t\n\r\f\v");
}
