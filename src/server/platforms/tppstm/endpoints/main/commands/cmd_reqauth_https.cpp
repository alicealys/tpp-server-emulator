#include <std_include.hpp>

#include "cmd_reqauth_https.hpp"
#include "cmd_abort_mother_base.hpp"

#include "database/auth.hpp"
#include "database/models/players.hpp"
#include "database/models/event_rankings.hpp"

namespace emulator::tpp
{
	void cmd_reqauth_https::wait_fail_delay(const std::chrono::system_clock::time_point start)
	{
		constexpr const auto duration = 50ms;
		const auto now = std::chrono::system_clock::now();
		const auto diff = now - start;

		if (diff < duration)
		{
			std::this_thread::sleep_for(duration - diff);
		}
	}

	nlohmann::json cmd_reqauth_https::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto start = std::chrono::system_clock::now();

		nlohmann::json result;

		auto& hash_val = data["hash"];
		auto& account_id_val = data["user_name"];

		if (!hash_val.is_string() || !account_id_val.is_string())
		{
			return error(ERR_INVALIDARG);
		}

		const auto hash = hash_val.get<std::string>();

		result["timeout_sec"] = database::vars.session_timeout.count();
		result["heartbeat_sec"] = database::vars.session_heartbeat.count();
		result["inquiry_id"] = 0;

		if (!player.has_value())
		{
			const auto auth_result = auth::authenticate_user(account_id_val, hash);
			if (!auth_result.success)
			{
				this->wait_fail_delay(start);
				return error(ERR_INVALID_ACCOUNT);
			}

			result["crypto_key"] = auth_result.crypto_key;
			result["session"] = auth_result.session_id;
			result["smart_device_id"] = auth_result.player->get_smart_device_id();
			result["user_id"] = auth_result.player->get_id();

			database::event_rankings::create_entries(auth_result.player->get_id());
			cmd_abort_mother_base::send_abort(auth_result.player.value());
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
			database::event_rankings::create_entries(player_found->get_id());
		}
		
		return result;
	}
}
