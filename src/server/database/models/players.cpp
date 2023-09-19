#include <std_include.hpp>

#include "players.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#define TABLE_DEF R"(
create table if not exists `players`
(
	id                  bigint unsigned	not null	auto_increment,
	account_id          bigint unsigned	not null,
	session_id          char(32)		default null,
	login_password      char(32)		default null,
	last_update         datetime		default null,
	crypto_key			char(32)		default null,
	ex_ip			    varchar(15)		default null,
	in_ip			    varchar(15)		default null,
	ex_port			    int unsigned	default 0,
	in_port			    int unsigned	default 0,
	nat					int unsigned	default 0,
	creation_time		datetime        default current_timestamp not null,
	primary key (`id`)
))"

namespace database::players
{
	namespace
	{
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

		std::vector<std::string> nat_types =
		{
			"SYMMETRIC_NAT",
			"RESTRICTED_PORT_CONE_NAT",
			"RESTRICTED_CONE_NAT",
			"OPEN_CHOICE_PORT",
			"FULL_CONE_NAT",
			"SYMMETRIC_OPEN",
			"SYMMETRIC_UDP_FIREWALL",
			"OPEN_INTERNET"
		};
	}

	std::uint32_t get_nat_type_id(const std::string& nat_type)
	{
		for (auto i = 0u; i < nat_types.size(); i++)
		{
			if (nat_type == nat_types[i])
			{
				return i;
			}
		}

		return 0;
	}

	std::string get_nat_type(const std::uint32_t nat_type_id)
	{
		if (nat_type_id < nat_types.size())
		{
			return nat_types[nat_type_id];
		}

		return nat_types[0];
	}

	auto players_table = players_table_t();

	std::optional<player> find(const std::uint64_t id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(players_table))
					.from(players_table)
						.where(players_table.id == id));

		if (results.empty())
		{
			return {};
		}

		const auto& row = results.front();
		return {player(row)};
	}

	std::optional<player> find_from_account(const std::uint64_t id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(players_table))
					.from(players_table)
						.where(players_table.account_id == id));

		if (results.empty())
		{
			return {};
		}

		const auto& row = results.front();
		return {player(row)};
	}

	std::optional<player> find_by_session_id(const std::string session_id)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(players_table))
					.from(players_table)
						.where(players_table.session_id == session_id));

		if (results.empty())
		{
			return {};
		}

		const auto& row = results.front();
		player player(row);
		return {player};
	}

	player find_or_insert(const std::uint64_t account_id)
	{
		{
			const auto found = find_from_account(account_id);
			if (found.has_value())
			{
				return found.value();
			}
		}

#pragma warning(push)
#pragma warning(disable: 4127)
		database::get()->operator()(
			sqlpp::insert_into(players_table)
				.set(players_table.account_id = account_id));
#pragma warning(pop)

		const auto found = find_from_account(account_id);
		if (!found.has_value())
		{
			throw std::runtime_error("[database::clients::insert] Insertion failed");
		}

		return found.value();
	}

	std::string generate_login_password(const std::uint64_t account_id)
	{
		const auto password = generate_data(16, false);

		database::get()->operator()(
			sqlpp::update(players_table)
				.set(players_table.login_password = password)
					.where(players_table.account_id == account_id));

		return password;
	}

	std::string generate_session_id(const std::uint64_t account_id)
	{
		const auto session_id = generate_data(16, false);

		database::get()->operator()(
			sqlpp::update(players_table)
				.set(players_table.session_id = session_id,
					 players_table.last_update = std::chrono::system_clock::now())
						.where(players_table.account_id == account_id));

		return session_id;
	}

	std::string generate_crypto_key(const std::uint64_t account_id)
	{
		const auto crypto_key = generate_data(16, true);

		database::get()->operator()(
			sqlpp::update(players_table)
				.set(players_table.crypto_key = crypto_key)
					.where(players_table.account_id == account_id));

		return crypto_key;
	}

	bool update_session(const std::uint64_t player_id)
	{
		const auto result = database::get()->operator()(
			sqlpp::update(players_table)	
				.set(players_table.last_update = std::chrono::system_clock::now())
					.where(players_table.id == player_id));

		return result != 0;
	}

	void set_ip_and_port(const std::uint64_t player_id, const std::string& ex_ip, const std::uint16_t ex_port,
		const std::string& in_ip, const std::uint16_t in_port, const std::string& nat_type)
	{
		const auto nat_type_id = get_nat_type_id(nat_type);
		database::get()->operator()(
			sqlpp::update(players_table)	
				.set(players_table.ex_ip = ex_ip,
					 players_table.in_ip = in_ip,
					 players_table.ex_port = ex_port,
					 players_table.in_port = in_port,
					 players_table.nat = nat_type_id)
					.where(players_table.id == player_id));
	}

	std::vector<player> get_player_list(const std::uint32_t limit)
	{
		auto results = database::get()->operator()(
			sqlpp::select(
				sqlpp::all_of(players_table))
					.from(players_table)
						.unconditionally().limit(limit));

		std::vector<player> list;

		for (auto& row : results)
		{
			list.emplace_back(row);
		}

		return list;
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}
	};
}

REGISTER_TABLE(database::players::table, 1)
