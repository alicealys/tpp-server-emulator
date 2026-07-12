#include <std_include.hpp>

#include "auth.hpp"
#include "database.hpp"

#include "component/console.hpp"

#include "models/players.hpp"
#include "models/steam_users.hpp"

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

		constexpr auto allow_list_file = "allow_list.json";
		constexpr auto deny_list_file = "deny_list.json";

		std::unordered_map<std::string, std::uint32_t> auth_mode_map =
		{
			{"offline", auth_offline},
			{"konami", auth_konami},
			{"custom", auth_custom},
			{"hybrid", auth_hybrid},
		};

		std::optional<std::unordered_set<std::uint64_t>> parse_list(const std::string& file)
		{
			std::string data;
			console::log("Parsing %s...\n", file.data());
			if (!utils::io::read_file(file, &data))
			{
				return {};
			}

			std::unordered_set<std::uint64_t> list;

			const auto json_list = nlohmann::json::parse(data, {}, false);
			if (json_list.is_discarded() || !json_list.is_array())
			{
				console::error("Error parsing list, must be a valid int64 array\n");
				return {list};
			}

			for (const auto& steam_id : json_list)
			{
				const auto id = steam_id.get<std::uint64_t>();
				console::log("Adding user \"%lli\" to list\n", id);
				list.insert(id);
			}

			return {list};
		}

		std::optional<std::unordered_set<std::uint64_t>>& get_allow_list()
		{
			static auto list = parse_list(allow_list_file);
			return list;
		}

		std::optional<std::unordered_set<std::uint64_t>>& get_deny_list()
		{
			static auto list = parse_list(deny_list_file);
			return list;
		}

		bool is_steam_id_allowed(const std::uint64_t steam_id)
		{
			const auto& allow_list = get_allow_list();

			if (!allow_list.has_value())
			{
				return true;
			}

			return allow_list->contains(steam_id);
		}

		bool is_steam_id_denied(const std::uint64_t steam_id)
		{
			const auto& deny_list = get_deny_list();

			if (!deny_list.has_value())
			{
				return false;
			}

			return deny_list->contains(steam_id);
		}

		bool can_authenticate(const std::uint64_t steam_id)
		{
			return !is_steam_id_denied(steam_id) && is_steam_id_allowed(steam_id);
		}

		std::optional<std::uint32_t> get_auth_mode()
		{
			const auto auth_mode_str = config::get<std::string>("auth_mode");
			const auto iter = auth_mode_map.find(auth_mode_str);
			if (iter == auth_mode_map.end())
			{
				return {};
			}

			return {iter->second};
		}
	}

	void initialize_lists()
	{
		get_allow_list();
		get_deny_list();
	}

	void reload_lists()
	{
		get_allow_list() = parse_list(allow_list_file);
		get_deny_list() = parse_list(deny_list_file);
	}

	std::optional<std::uint64_t> verify_ticket_konami(const std::string& auth_ticket, const size_t ticket_size, const bool is_tpp)
	{
		nlohmann::json data;
		data["steam_ticket"] = utils::encoding::split_into_lines(auth_ticket);
		data["steam_ticket_size"] = ticket_size;
		data["region"] = 4;
		data["msgid"] = "CMD_AUTH_STEAMTICKET";
		data["rqid"] = 0;
		data["lang"] = "en";
		data["country"] = "ww";

		const auto end_point = is_tpp ? "tppstm/main" : "mgostm/main";
		auto result_opt = client.send_command(end_point, data, false);
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

	std::optional<std::uint64_t> verify_ticket_custom(const std::string& auth_ticket, const size_t ticket_size)
	{
		const auto data = utils::cryptography::base64::decode(auth_ticket);
		if (data.size() < sizeof(auth_ticket_custom_t))
		{
			return {};
		}

		const auto ticket = reinterpret_cast<const auth_ticket_custom_t*>(data.data());
		const auto token = std::string{ticket->auth_token, sizeof(auth_ticket_custom_t::auth_token)};

		const auto is_valid = std::ranges::all_of(token.begin(), token.end(), [](const char c)
		{
			return std::isalnum(static_cast<int>(c));
		});

		if (!is_valid)
		{
			return {};
		}

		if (database::steam_users::authenticate(ticket->account_id, token))
		{
			return {ticket->account_id};
		}

		return {};
	}

	std::optional<std::uint64_t> verify_ticket(const std::string& auth_ticket, const size_t ticket_size, const bool is_tpp)
	{
		const auto auth_mode = get_auth_mode();
		if (!auth_mode.has_value())
		{
			return {};
		}

		switch (auth_mode.value())
		{
		case auth_offline:
			return verify_ticket_offline(auth_ticket, ticket_size);
		case auth_konami:
			return verify_ticket_konami(auth_ticket, ticket_size, is_tpp);
		case auth_custom:
			return verify_ticket_custom(auth_ticket, ticket_size);
		case auth_hybrid:
		{
			if (ticket_size == sizeof(auth_ticket_custom_t))
			{
				return verify_ticket_custom(auth_ticket, ticket_size);
			}
			else
			{
				return verify_ticket_konami(auth_ticket, ticket_size, is_tpp);
			}

			return {};
		}
		}

		return {};
	}

	std::optional<auth_ticket_response> authenticate_user_with_ticket(const std::string& auth_ticket, const size_t ticket_size, const bool is_tpp)
	{
		const auto account_id_opt = verify_ticket(auth_ticket, ticket_size, is_tpp);
		if (!account_id_opt.has_value())
		{
			return {};
		}

		const auto account_id = account_id_opt.value();
		if (!can_authenticate(account_id))
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

	std::string generate_data(const size_t len, bool base64)
	{
		const auto data = utils::cryptography::random::get_data(len);
		if (base64)
		{
			return utils::cryptography::base64::encode(data);
		}
		else
		{
			return utils::string::dump_hex(data, "", false);
		}
	}

	bool validate_auth_mode(const nlohmann::json& value)
	{
		if (!value.is_string())
		{
			return false;
		}

		const auto value_str = value.get<std::string>();
		return auth_mode_map.contains(value_str);
	}
}
