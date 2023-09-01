#pragma once

namespace database
{
	std::string generate_crypto_key(const std::uint64_t steam_id);
	std::string get_crypto_key(const std::string& session_id);

	std::string generate_login_password(const std::string& account_id);
	std::string get_smart_device_id(const std::string& account_id);
}
