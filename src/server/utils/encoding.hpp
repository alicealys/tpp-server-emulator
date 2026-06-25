#pragma once

namespace utils::encoding
{
	std::string split_into_lines(const std::string& string, const size_t chars_per_line = 76);
	std::string unescape_json(const std::string& str);
	std::string decode_url_string(const std::string& str);

	std::string encode_as_hex(const char* buffer, const size_t size);
	std::string encode_as_hex(const std::string& data);

	template <typename T>
	std::string encode_binary(const T& data)
	{
		const auto buffer = reinterpret_cast<char*>(reinterpret_cast<size_t>(&data));
		return encode_as_hex(buffer, sizeof(T));
	}
}
