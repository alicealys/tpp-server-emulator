#include <std_include.hpp>

#include "cmd_auth_steamticket.hpp"

#include "database/auth.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_auth_steamticket::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["xuid"] = {};

		const auto& steam_ticket_val = data["steam_ticket"];
		if (!steam_ticket_val.is_string())
		{
			result["result"] = "ERR_INVALID_TICKET";
			return result;
		}

		if (session_key.empty())
		{
			const auto steam_ticket = steam_ticket_val.get<std::string>();
			const auto auth_result_opt = auth::authenticate_user_with_ticket(steam_ticket);
			if (!auth_result_opt.has_value())
			{
				result["result"] = "ERR_INVALID_TICKET";
				return result;
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
			const auto player = database::players::find_by_session_id(session_key);
			if (!player.has_value())
			{
				result["result"] = "ERR_INVALID_SESSION";
				return result;
			}

			result["account_id"] = std::to_string(player->get_account_id());
			result["currency"] = player->get_currency();
			result["loginid_password"] = player->get_login_password();
			result["smart_device_id"] = player->get_smart_device_id();
			result["result"] = "NOERR";
		}

		return result;
	}
}
