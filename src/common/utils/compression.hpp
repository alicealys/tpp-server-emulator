#pragma once

#include <string>
#include <unordered_map>

#define CHUNK 16384u

namespace utils::compression
{
	namespace zlib
	{
		std::string compress(const std::string& data, const std::uint32_t extra_bound = 0u);
		std::string decompress(const std::string& data);
	}
};
