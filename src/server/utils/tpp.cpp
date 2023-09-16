#include <std_include.hpp>

#include "tpp.hpp"

#include <utils/string.hpp>

namespace utils::tpp
{
	namespace
	{
		// https://github.com/unknown321/mgsv_emulator/blob/master/static_key.bin
		std::uint8_t static_key[16] = {0xD8, 0x89, 0x0A, 0xF0, 0x66, 0xC9, 0x6B, 0x40, 0xD7, 0x01, 0xAE, 0xFC, 0x43, 0x6F, 0xF9, 0xFE};
	}

	std::uint8_t* get_static_key()
	{
		return static_key;
	}

	std::size_t get_static_key_len()
	{
		return sizeof(static_key);
	}

	std::uint32_t calculate_mb_coins(const std::uint32_t seconds)
	{
		return static_cast<std::uint32_t>(static_cast<float>(seconds) * 0.01565f);
	}
}
