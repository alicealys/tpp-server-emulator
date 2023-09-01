#pragma once

namespace utils::encoding
{
	std::string split_into_lines(const std::string& string, const size_t chars_per_line = 76);
	std::string unescape_json(const std::string& str);
	std::string decode_url_string(const std::string& str);
}
