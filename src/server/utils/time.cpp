#include <std_include.hpp>

#include "time.hpp"
#include "database/vars.hpp"

namespace utils::time
{
	std::chrono::seconds get_reference_time()
	{
		return database::vars.reference_time;
	}

	void get_section(const std::chrono::seconds& duration, std::chrono::seconds& start, std::chrono::seconds& end, std::size_t& index)
	{
		const auto ref_time = get_reference_time();
		const auto now = std::chrono::system_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::seconds>((now - ref_time).time_since_epoch());
		index = diff / duration;

		start = ref_time + index * duration;
		end = start + duration;
	}

	void get_section(const std::chrono::seconds& duration, std::chrono::seconds& start, std::chrono::seconds& end)
	{
		std::size_t index{};
		get_section(duration, start, end, index);
	}

	std::size_t get_day()
	{
		const auto ref_time = get_reference_time();
		const auto now = std::chrono::system_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::seconds>((now - ref_time).time_since_epoch());
		return std::chrono::floor<std::chrono::days>(diff).count();
	}

	std::chrono::seconds get_week_begin()
	{
		std::chrono::seconds start{};
		std::chrono::seconds end{};
		get_section(24h * 7, start, end);
		return start;
	}

	std::chrono::seconds get_day_begin()
	{
		std::chrono::seconds start{};
		std::chrono::seconds end{};
		get_section(24h, start, end);
		return start;
	}

	std::size_t get_weekday()
	{
		return get_day() % 7;
	}

	std::size_t get_week()
	{
		return get_day() / 7;
	}

	std::chrono::seconds from_now(const std::chrono::seconds duration)
	{
		const auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::seconds>((now + duration).time_since_epoch());
	}

	std::chrono::seconds from_now(const std::chrono::day duration)
	{
		return from_now(duration.operator unsigned int() * 24h);
	}

	std::uint32_t version(const std::chrono::seconds time)
	{
		const std::chrono::system_clock::time_point tp{time};
		const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(tp)};

		return
			static_cast<std::uint32_t>(static_cast<std::int32_t>(ymd.year())) * 10000u +
			static_cast<std::uint32_t>(ymd.month()) * 100u +
			static_cast<std::uint32_t>(ymd.day());
	}
}
