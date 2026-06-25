#include <std_include.hpp>

#include "encoding.hpp"

#include <utils/string.hpp>

namespace utils::encoding
{
	std::string split_into_lines(const std::string& string, const size_t chars_per_line)
	{
		std::string encoded;

		for (size_t i = 0ull; i < string.size(); i++)
		{
			encoded += string[i];
			if (((i + 1) % chars_per_line) == 0)
			{
				encoded.append("\r\n");
			}
		}

		encoded.append("\r\n");

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

	std::string encode_as_hex(const char* buffer, const size_t size)
	{
		std::string encoded;
		encoded.resize(size * 2 + 3);

		auto out_buffer = encoded.data();
		out_buffer[0] = 'x';
		out_buffer[1] = '\'';
		out_buffer += 2;

		for (auto i = 0u; i < size; i++)
		{
			char hex[3]{};
			snprintf(hex, 3, "%02X", static_cast<std::uint8_t>(buffer[i]));
			out_buffer[0] = hex[0];
			out_buffer[1] = hex[1];
			out_buffer += 2;
		}
		out_buffer[0] = '\'';

		return encoded;
	}

	std::string encode_as_hex(const std::string& data)
	{
		return encode_as_hex(data.data(), data.size());
	}
}
