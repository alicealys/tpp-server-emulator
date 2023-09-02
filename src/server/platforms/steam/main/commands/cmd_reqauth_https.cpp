#include <std_include.hpp>

#include "cmd_reqauth_https.hpp"

#include "database/auth.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_reqauth_https::execute(const nlohmann::json& data)
	{
		nlohmann::json result;
		result["xuid"] = {};

		const auto& hash_val = data["hash"];
		const auto& account_id_val = data["user_name"];
		if (!hash_val.is_string() || !account_id_val.is_string())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto hash = hash_val.get<std::string>();
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
		result["timeout_sec"] = 200;
		result["heartbeat_sec"] = 60;
		result["inquiry_id"] = 0;
		result["result"] = "NOERR";

		return result;
	}
}
