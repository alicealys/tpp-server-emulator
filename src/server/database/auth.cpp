#include <std_include.hpp>

#include "auth.hpp"
#include "database.hpp"

#include <utils/string.hpp>

namespace auth
{
	std::optional<std::string> verify_ticket(const std::string& auth_ticket)
	{
		std::string data;
		data.resize(18);
		return utils::string::dump_hex(data);
	}

	std::optional<auth_ticket_response> authenticate_user(const std::string& auth_ticket)
	{
		const auto account_id_opt = verify_ticket(auth_ticket);
		if (!account_id_opt.has_value())
		{
			return {};
		}

		auth_ticket_response response{};
		response.account_id = account_id_opt.value();
		response.currency = "EUR";
		response.password = database::generate_login_password(response.account_id);
		response.smart_device_id = database::get_smart_device_id(response.account_id);

		return {response};
	}
}
