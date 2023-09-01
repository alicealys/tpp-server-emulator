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

	std::optional<auth_ticket_response> authenticate_user(const std::string& auth_ticket);
}
