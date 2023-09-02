#include <std_include.hpp>

#include "auth.hpp"
#include "database.hpp"

#include "models/players.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace auth
{
	std::optional<std::uint64_t> verify_ticket(const std::string& auth_ticket)
	{
		const auto data = utils::cryptography::base64::decode(auth_ticket);
		const auto data_ptr = reinterpret_cast<size_t>(data.data());
		const auto account_id = *reinterpret_cast<std::uint64_t*>(data_ptr + 12);
		return {account_id};
	}

	std::optional<auth_ticket_response> authenticate_user_with_ticket(const std::string& auth_ticket)
	{
		const auto account_id_opt = verify_ticket(auth_ticket);
		if (!account_id_opt.has_value())
		{
			return {};
		}

		const auto account_id = account_id_opt.value();

		database::players::find_or_insert(account_id);

		auth_ticket_response response{};
		response.account_id = std::to_string(account_id);
		response.currency = "EUR";
		response.password = database::players::generate_login_password(account_id);
		response.smart_device_id = database::get_smart_device_id(response.account_id);

		return {response};
	}

	std::optional<auth_response> authenticate_user(const std::string& account_id, const std::string& password)
	{
		const auto account_id_int = std::strtoull(account_id.data(), nullptr, 10);
		const auto player_opt = database::players::find_from_account(account_id_int);
		if (!player_opt.has_value())
		{
			return {};
		}

		const auto& player = player_opt.value();

		auth_response response{};

		const auto pwd = player.get_login_password();
		const auto hash = utils::cryptography::md5::compute(pwd);
		const auto hash_b64 = utils::cryptography::base64::encode(hash);

		response.success = password == hash_b64;
		if (!response.success)
		{
			return {response};
		}

		response.player_id = player.get_id();
		response.smart_device_id = database::get_smart_device_id(account_id);
		response.session_id = database::players::generate_session_id(account_id_int);
		response.crypto_key = database::players::generate_crypto_key(account_id_int);

		return {response};
	}
}
