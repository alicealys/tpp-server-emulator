#pragma once

namespace auth
{
	struct auth_ticket_response
	{
		std::string account_id;
		std::string currency;
		std::string password;
		std::string smart_device_id;
	};

	struct auth_response
	{
		bool success;
		std::uint64_t player_id;
		std::string session_id;
		std::string crypto_key;
		std::string smart_device_id;
	};

	struct auth_ticket_custom_t
	{
		std::uint64_t account_id;
		char auth_token[32];
	};

	enum auth_mode_t
	{
		auth_offline = 0,
		auth_konami = 1,
		auth_custom = 2,
		auth_hybrid = 3
	};

	void initialize_lists();
	void reload_lists();

	std::optional<auth_ticket_response> authenticate_user_with_ticket(const std::string& auth_ticket, const size_t ticket_size, const bool is_tpp);
	std::optional<auth_response> authenticate_user(const std::string& account_id, const std::string& password);
	std::string generate_data(const size_t len, bool base64);
	bool validate_auth_mode(const nlohmann::json& value);
}
