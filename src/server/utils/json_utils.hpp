#pragma once

#include <utils/cryptography.hpp>
#include "encoding.hpp"

namespace utils::json_utils
{
	template <typename T>
	bool is(const nlohmann::json& value);

	template <typename T>
	void get_or(const nlohmann::json& value, T& dest, const T& default_value = {})
	{
		if (!is<T>(value))
		{
			dest = default_value;
		}
		else
		{
			dest = value.get<T>();
		}
	}

	template <typename T>
	bool parse_array(nlohmann::json& src, T& dest, const bool strict = true)
	{
		if (!src.is_array() || (strict && src.size() != ARRAYSIZE(dest)))
		{
			return false;
		}

		for (auto i = 0ull; i < src.size(); i++)
		{
			get_or(src[i], dest[i]);
		}

		return true;
	}

	template <typename T, typename F>
	bool parse_array(nlohmann::json& src, T& dest, F&& fn, const bool strict = true)
	{
		if (!src.is_array() || (strict && src.size() != ARRAYSIZE(dest)))
		{
			return false;
		}

		const auto count = std::min(src.size(), ARRAYSIZE(dest));
		for (auto i = 0ull; i < count; i++)
		{
			fn(dest[i], src[i]);
		}

		return true;
	}

	template <std::size_t N>
	bool parse_base64(nlohmann::json& src, std::uint8_t(&dest)[N], const bool strict = false, const bool or_bytes = false)
	{
		if (!src.is_string())
		{
			return false;
		}

		auto data = utils::encoding::decode_url_string(src.get<std::string>());
		data = utils::cryptography::base64::decode(data);

		if ((strict && data.size() != N) || (!strict && data.size() > N))
		{
			return false;
		}

		if (or_bytes)
		{
			for (auto i = 0ull; i < data.size(); i++)
			{
				dest[i] |= data[i];
			}
		}
		else
		{
			std::memcpy(dest, data.data(), data.size());
		}

		return true;
	}

	template <std::size_t N>
	bool parse_string(nlohmann::json& src, char(&dest)[N])
	{
		if (!src.is_string())
		{
			return false;
		}

		const auto& string = src.get<std::string>();
		if (string.size() > N - 1)
		{
			return false;
		}

		std::memcpy(dest, string.data(), string.size());
		return true;
	}
}
