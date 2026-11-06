#include <std_include.hpp>

#include "json_utils.hpp"

namespace utils::json_utils
{
	template <>
	bool is<std::int8_t>(const nlohmann::json& value)
	{
		return value.is_number_integer();
	}

	template <>
	bool is<std::int16_t>(const nlohmann::json& value)
	{
		return value.is_number_integer();
	}

	template <>
	bool is<std::int32_t>(const nlohmann::json& value)
	{
		return value.is_number_integer();
	}

	template <>
	bool is<std::int64_t>(const nlohmann::json& value)
	{
		return value.is_number_integer();
	}

	template <>
	bool is<std::uint8_t>(const nlohmann::json& value)
	{
		return value.is_number_unsigned();
	}

	template <>
	bool is<std::uint16_t>(const nlohmann::json& value)
	{
		return value.is_number_unsigned();
	}

	template <>
	bool is<std::uint32_t>(const nlohmann::json& value)
	{
		return value.is_number_unsigned();
	}

	template <>
	bool is<std::uint64_t>(const nlohmann::json& value)
	{
		return value.is_number_unsigned();
	}

	template <>
	bool is<std::string>(const nlohmann::json& value)
	{
		return value.is_string();
	}

	template <>
	bool is<bool>(const nlohmann::json& value)
	{
		return value.is_boolean();
	}

	template <>
	bool is<float>(const nlohmann::json& value)
	{
		return value.is_number_float();
	}
}
