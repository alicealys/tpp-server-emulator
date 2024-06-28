#include <std_include.hpp>

#include "cmd_reqauth_https.hpp"

#include "database/auth.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_reqauth_https::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& hash_val = data["hash"];
		const auto& account_id_val = data["user_name"];
		if (!hash_val.is_string() || !account_id_val.is_string())
		{
			return error(ERR_INVALIDARG);
		}

		const auto hash = hash_val.get<std::string>();

		result["timeout_sec"] = std::chrono::duration_cast<std::chrono::seconds>(database::players::session_timeout).count();
		result["heartbeat_sec"] = std::chrono::duration_cast<std::chrono::seconds>(database::players::session_heartbeat).count();
		result["inquiry_id"] = 0;

		if (!player.has_value())
		{
			const auto auth_result_opt = auth::authenticate_user(account_id_val, hash);
			if (!auth_result_opt.has_value())
			{
				return error(ERR_INVALID_ACCOUNT);
			}

			const auto& auth_result = auth_result_opt.value();
			result["crypto_key"] = auth_result.crypto_key;
			result["session"] = auth_result.session_id;
			result["smart_device_id"] = auth_result.smart_device_id;
			result["user_id"] = auth_result.player_id;
			result["result"] = "NOERR";

			database::players::abort_mother_base(auth_result.player_id);
		}
		else
		{
			auto expired = false;
			const auto session_key = player->get_session_id();
			const auto player_found = database::players::find_by_session_id(session_key, false, &expired);

			if (!player_found.has_value())
			{
				return error(ERR_INVALID_SESSION);
			}

			if (expired)
			{
				database::players::update_session(player_found.value());
			}

			result["crypto_key"] = player_found->get_crypto_key();
			result["session"] = session_key;
			result["smart_device_id"] = player_found->get_smart_device_id();
			result["user_id"] = player_found->get_id();
		}

		return result;
	}
}
