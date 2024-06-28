#include <std_include.hpp>

#include "cmd_auth_steamticket.hpp"

#include "database/auth.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_auth_steamticket::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& steam_ticket_val = data["steam_ticket"];
		const auto& steam_ticket_size_j = data["steam_ticket_size"];
		if (!steam_ticket_val.is_string() || !steam_ticket_size_j.is_number_unsigned())
		{
			return error(ERR_INVALID_TICKET);
		}

		if (!player.has_value())
		{
			const auto steam_ticket = steam_ticket_val.get<std::string>();
			const auto steam_ticket_size = steam_ticket_size_j.get<std::size_t>();

			const auto auth_result_opt = auth::authenticate_user_with_ticket(steam_ticket, steam_ticket_size);
			if (!auth_result_opt.has_value())
			{
				return error(ERR_INVALID_TICKET);
			}

			const auto& auth_result = auth_result_opt.value();
			result["account_id"] = auth_result.account_id;
			result["currency"] = auth_result.currency;
			result["loginid_password"] = auth_result.password;
			result["smart_device_id"] = auth_result.smart_device_id;
			result["result"] = "NOERR";
		}
		else
		{
			result["account_id"] = std::to_string(player->get_account_id());
			result["currency"] = player->get_currency();
			result["loginid_password"] = player->get_login_password();
			result["smart_device_id"] = player->get_smart_device_id();
			result["result"] = "NOERR";

			database::players::update_session(player.value());
		}

		return result;
	}
}
