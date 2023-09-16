#pragma once

namespace utils::tpp
{
	std::uint8_t* get_static_key();
	std::size_t get_static_key_len();
	std::uint32_t calculate_mb_coins(const std::uint32_t seconds);
}
