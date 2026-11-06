#include <std_include.hpp>

#include "random.hpp"

namespace utils::random
{
	std::mt19937& get_engine()
	{
		static auto engine = []()
		{
			std::random_device rnd;
			std::mt19937 mt(rnd());
			return mt;
		}();

		return engine;
	}

	std::uint64_t get_integer(const std::uint64_t min, const std::uint64_t max)
	{
		std::uniform_int_distribution<std::uint64_t> distribution(min, max);
		return distribution(get_engine());
	}

	bool chance(const std::uint32_t probability)
	{
		std::bernoulli_distribution distribution(static_cast<float>(probability) / 100.f);
		return distribution(get_engine());
	}

	bool cointoss()
	{
		return chance(50u);
	}
}
