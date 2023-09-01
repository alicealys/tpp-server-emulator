#include <std_include.hpp>

#include "database.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace database
{
	std::string get_default_crypto_key()
	{
		std::string data;
		data.resize(16);
		return utils::cryptography::base64::encode(data);
	}

	std::string generate_crypto_key(const std::uint64_t steam_id)
	{
		return get_default_crypto_key();
	}

	std::string get_crypto_key(const std::string& session_id)
	{
		return get_default_crypto_key();
	}

	std::string generate_login_password(const std::string& account_id)
	{
		std::string data;
		data.resize(32);
		return utils::string::dump_hex(data);
	}

	std::string get_smart_device_id(const std::string& account_id)
	{
		std::string data;
		data.resize(160);
		return utils::cryptography::base64::encode(data);
	}
}
