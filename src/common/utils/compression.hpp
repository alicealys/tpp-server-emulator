#pragma once

#include <string>
#include <unordered_map>
#include <zlib.h>

#define CHUNK 16384u

namespace utils::compression
{
	namespace zlib
	{
		std::string compress(const std::string& data, const std::uint32_t level = Z_DEFAULT_COMPRESSION, const bool null_term = false);
		std::string decompress(const std::string& data);
	}
};
