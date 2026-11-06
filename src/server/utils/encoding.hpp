#pragma once

#include "http_server.hpp"
#include <utils/cryptography.hpp>

namespace utils::encoding
{
	std::string split_into_lines(const std::string& string, const size_t chars_per_line = 76);
	std::string unescape_json(const std::string& str);
	std::string decode_url_string(const std::string& str);

	inline void byte_to_hex(const std::uint8_t value, char* out)
	{
		static const char* map = "0123456789ABCDEF";
		out[1] = map[value & 0xF];
		out[0] = map[(value >> 4) & 0xF];
	}

	std::string encode_as_hex(const char* buffer, const size_t size);
	std::string encode_as_hex(const std::string& data);

	template <typename T>
	inline void encode_as_hex_single(const T* data, char* out)
	{
		const auto buffer = reinterpret_cast<const std::uint8_t*>(data);
		for (auto i = 0ull; i < sizeof(T); i++)
		{
			byte_to_hex(buffer[i], out);
			out += 2;
		}
	}

	template <typename T>
	std::string encode_binary(const T& data)
	{
		const auto buffer = reinterpret_cast<char*>(reinterpret_cast<size_t>(&data));
		return encode_as_hex(buffer, sizeof(T));
	}

	template <typename T>
	std::string encode_base64(const T& data)
	{
		const auto buffer = reinterpret_cast<std::uint8_t*>(reinterpret_cast<size_t>(&data));
		return utils::cryptography::base64::encode(buffer, sizeof(T));
	}

	std::string validate_base64(const std::string& data);
}
