#include <std_include.hpp>

#include "cmd_reqauth_https.hpp"

#include "database/auth.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_reqauth_https::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto& hash_val = data["hash"];
		const auto& account_id_val = data["user_name"];
		if (!hash_val.is_string() || !account_id_val.is_string())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto hash = hash_val.get<std::string>();

		result["timeout_sec"] = std::chrono::duration_cast<std::chrono::seconds>(database::players::session_timeout).count();
		result["heartbeat_sec"] = std::chrono::duration_cast<std::chrono::seconds>(database::players::session_heartbeat).count();
		result["inquiry_id"] = 0;

		if (session_key.empty())
		{
			const auto auth_result_opt = auth::authenticate_user(account_id_val, hash);
			if (!auth_result_opt.has_value())
			{
				result["result"] = "ERR_INVALID_ACCOUNT";
				return result;
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
			const auto player = database::players::find_by_session_id(session_key, false, &expired);
			if (!player.has_value())
			{
				result["result"] = "ERR_INVALID_SESSION";
				return result;
			}

			if (expired)
			{
				database::players::update_session(player.value());
			}

			result["crypto_key"] = player->get_crypto_key();
			result["session"] = session_key;
			result["smart_device_id"] = player->get_smart_device_id();
			result["user_id"] = player->get_id();
		}

		return result;
	}
}
