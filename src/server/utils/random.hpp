#pragma once

namespace utils::random
{
	std::mt19937& get_engine();

	std::uint64_t get_integer(const std::uint64_t min, const std::uint64_t max);

	template <typename T>
	T get_integer(const T min, const T max)
	{
		return static_cast<T>(get_integer(static_cast<std::uint64_t>(min), static_cast<std::uint64_t>(max)));
	}

	template <typename T>
	std::size_t index(const T& container)
	{
		if (container.empty())
		{
			throw std::runtime_error("[random::index] container is empty");
		}

		return get_integer<std::size_t>(0, container.size() - 1);
	}

	bool chance(const std::uint32_t probability);
	bool cointoss();
}
