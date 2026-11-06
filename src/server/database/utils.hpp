#pragma once

#include "utils/encoding.hpp"

namespace database
{
	inline auto nullable_integer(const std::uint64_t value)
	{
		if (value == 0ull)
		{
			return sqlpp::value_or_null<sqlpp::integer_unsigned>(sqlpp::null);
		}
		else
		{
			return sqlpp::value_or_null(value);
		}
	}

	template <typename T>
	class database_struct : public T
	{
	public:
		std::string serialize() const
		{
			return utils::encoding::encode_base64(*this);
		}

		void deserialize(const std::string& data)
		{
			std::memset(this, 0, sizeof(T));

			const auto deserialized = utils::cryptography::base64::decode(data);
			if (deserialized.size() != sizeof(T))
			{
				return;
			}

			std::memcpy(this, deserialized.data(), deserialized.size());
		}
	};
}
