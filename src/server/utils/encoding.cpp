#include <std_include.hpp>

#include "encoding.hpp"

#include <utils/string.hpp>

namespace utils::encoding
{
	std::string split_into_lines(const std::string& string, const size_t chars_per_line)
	{
		std::string encoded;

		for (auto i = 0; i < string.size(); i++)
		{
			encoded += string[i];
			if (((i + 1) % chars_per_line) == 0)
			{
				encoded += "\r\n";
			}
		}

		encoded += "\r\n";

		return encoded;
	}

	std::string unescape_json(const std::string& str)
	{
		auto result = utils::string::replace(str, "\\\\r\\\\n", "");
		result = utils::string::replace(result, "\\r\\n", "");
		result = utils::string::replace(result, "\\", "");
		result = utils::string::replace(result, "\"{", "{");
		result = utils::string::replace(result, "}\"", "}");
		return result;
	}

	std::string decode_url_string(const std::string& str)
	{
		auto result = utils::string::replace(str, "%2B", "+");
		result = utils::string::replace(result, "\r\n", "");
		return result;
	}
}
