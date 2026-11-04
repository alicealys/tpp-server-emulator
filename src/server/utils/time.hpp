#pragma once
#include <sqlpp11/sqlpp11.h>

namespace utils::time
{
	std::chrono::seconds get_week_begin();
	std::chrono::seconds get_day_begin();
	void get_section(const std::chrono::seconds& duration, std::chrono::seconds& start, std::chrono::seconds& end, std::size_t& index);
	void get_section(const std::chrono::seconds& duration, std::chrono::seconds& start, std::chrono::seconds& end);

	std::size_t get_day();
	std::size_t get_weekday();
	std::size_t get_week();

	std::chrono::seconds from_now(const std::chrono::seconds duration);
	std::chrono::seconds from_now(const std::chrono::day duration);

	std::uint32_t version(const std::chrono::seconds);
}
