#include <std_include.hpp>

#include "auth.hpp"
#include "database.hpp"

#include "component/console.hpp"

#include "models/players.hpp"

#include "utils/tpp_client.hpp"
#include "utils/encoding.hpp"
#include "utils/config.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>
#include <utils/io.hpp>

namespace auth
{
	namespace
	{
		utils::tpp::tpp_client client;

		std::optional<std::unordered_set<std::uint64_t>> parse_allow_list()
		{
			std::string data;
			if (!utils::io::read_file("allow_list.json", &data))
			{
				return {};
			}

			std::unordered_set<std::uint64_t> list;

			const auto json_list = nlohmann::json::parse(data, {}, false);
			if (json_list.is_discarded() || !json_list.is_array())
			{
				console::error("Error parsing allow list, must be a valid int64 array\n");
				return {list};
			}

			for (const auto& steam_id : json_list)
			{
				list.insert(steam_id.get<std::uint64_t>());
			}

			return {list};
		}

		std::optional<std::unordered_set<std::uint64_t>> get_allow_list()
		{
			static const auto list = parse_allow_list();
			return list;
		}

		bool is_steam_id_allowed(const std::uint64_t steam_id)
		{
			const auto allow_list = get_allow_list();
			if (!allow_list.has_value())
			{
				return true;
			}

			return allow_list->contains(steam_id);
		}
	}

	std::optional<std::uint64_t> verify_ticket_konami(const std::string& auth_ticket, const size_t ticket_size)
	{
		nlohmann::json data;
		data["steam_ticket"] = utils::encoding::split_into_lines(auth_ticket);
		data["steam_ticket_size"] = ticket_size;
		data["region"] = 4;
		data["msgid"] = "CMD_AUTH_STEAMTICKET";
		data["rqid"] = 0;
		data["lang"] = "en";
		data["country"] = "ww";

		auto result_opt = client.send_command("tppstm/main", data, false);
		if (!result_opt.has_value())
		{
			return {};
		}

		auto& resp = result_opt.value();
		auto& result = resp["data"];
		if (!result.is_object())
		{
			return {};
		}

		auto& error = result["result"];
		if (error != "NOERR")
		{
			return {};
		}

		auto& account_id_j = result["account_id"];
		if (!account_id_j.is_string())
		{
			return {};
		}

		const auto account_id_str = account_id_j.get<std::string>();
		const auto account_id = std::strtoull(account_id_str.data(), nullptr, 10);

		if (account_id == 0ull)
		{
			return {};
		}

		return {account_id};
	}

	std::optional<std::uint64_t> verify_ticket_offline(const std::string& auth_ticket, const size_t ticket_size)
	{
		const auto data = utils::cryptography::base64::decode(auth_ticket);
		if (data.size() < 20)
		{
			return {};
		}

		const auto data_ptr = reinterpret_cast<size_t>(data.data());
		const auto account_id = *reinterpret_cast<std::uint64_t*>(data_ptr + 12);
		return {account_id};
	}

	std::optional<std::uint64_t> verify_ticket(const std::string& auth_ticket, const size_t ticket_size)
	{
		static const auto use_konami_auth = config::get<bool>("use_konami_auth");
		if (use_konami_auth)
		{
			return verify_ticket_konami(auth_ticket, ticket_size);
		}
		else
		{
			return verify_ticket_offline(auth_ticket, ticket_size);
		}
	}

	std::optional<auth_ticket_response> authenticate_user_with_ticket(const std::string& auth_ticket, const size_t ticket_size)
	{
		const auto account_id_opt = verify_ticket(auth_ticket, ticket_size);
		if (!account_id_opt.has_value())
		{
			return {};
		}

		const auto account_id = account_id_opt.value();
		if (!is_steam_id_allowed(account_id))
		{
			console::log("Denying user \"%lli\"\n", account_id);
			return {};
		}

		console::log("Allowing user \"%lli\"\n", account_id);

		const auto player = database::players::find_or_insert(account_id);

		auth_ticket_response response{};
		response.account_id = std::to_string(account_id);
		response.currency = player.get_currency();
		response.password = database::players::generate_login_password(account_id);
		response.smart_device_id = player.get_smart_device_id();

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
		response.smart_device_id = player.get_smart_device_id();
		response.session_id = database::players::generate_session_id(account_id_int);
		response.crypto_key = database::players::generate_crypto_key(account_id_int);

		return {response};
	}
}
