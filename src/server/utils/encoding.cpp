#include <std_include.hpp>

#include "encoding.hpp"

#include <utils/string.hpp>

namespace utils::encoding
{
	std::string split_into_lines(const std::string& string, const size_t chars_per_line)
	{
		std::string encoded;
		encoded.reserve(string.size() + ((string.size() / chars_per_line) + 1) * 2);

		for (auto i = 0ull; i < string.size(); i++)
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

#define CHECK_SUBSTR(s) i + (sizeof(s) - 2) < str.size() && !std::strncmp(&str[i], s, sizeof(s) - 1)

	std::string unescape_json(const std::string& str)
	{
		std::string decoded;
		decoded.resize(str.size());

		auto idx = 0ull;
		for (auto i = 0u; i < str.size(); )
		{
			if (CHECK_SUBSTR("\\\\r\\\\n"))
			{
				i += 6;
			}
			else if (CHECK_SUBSTR("\\r\\n"))
			{
				i += 4;
			}
			else if (CHECK_SUBSTR("\\"))
			{
				i += 1;
			}
			else if (CHECK_SUBSTR("\"{"))
			{
				decoded[idx++] = '{';
				i += 2;
			}
			else if (CHECK_SUBSTR("}\""))
			{
				decoded[idx++] = '}';
				i += 2;
			}
			else
			{
				decoded[idx++] = str[i];
				++i;
			}
		}

		decoded.resize(idx);
		return decoded;
	}

	std::string decode_url_string(const std::string& str)
	{
		std::string decoded;
		decoded.resize(str.size());

		auto idx = 0ull;
		for (auto i = 0u; i < str.size(); )
		{
			if (CHECK_SUBSTR("%2B"))
			{
				decoded[idx++] = '+';
				i += 3;
			}
			else if (i + 1 < str.size() && !std::strncmp(&str[i], "\r\n", 2))
			{
				i += 2;
			}
			else
			{
				decoded[idx++] = str[i];
				++i;
			}
		}

		decoded.resize(idx);
		return decoded;
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
			std::snprintf(hex, 3, "%02X", static_cast<std::uint8_t>(buffer[i]));
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
