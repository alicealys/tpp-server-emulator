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
	last_update         date			default null,
	crypto_key			char(32)		default null,
	creation_time		datetime        default current_timestamp not null,
	primary key (`id`)
))"

namespace database::players
{
	auto players_table = players_t();

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
		return {player(row)};
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
		const auto data = utils::cryptography::random::get_data(16);
		const auto password = utils::string::dump_hex(data, "", false);

		const auto result = database::get()->operator()(
			sqlpp::update(players_table)
				.set(players_table.login_password = password)
					.where(players_table.account_id == account_id));

		printf("password: %s %lli\n", password.data(), result);

		return password;
	}

	std::string generate_session_id(const std::uint64_t account_id)
	{
		const auto data = utils::cryptography::random::get_data(16);
		const auto session_id = utils::string::dump_hex(data, "", false);

		const auto result = database::get()->operator()(
			sqlpp::update(players_table)
				.set(players_table.session_id = session_id,
					 players_table.last_update = std::chrono::system_clock::now())
						.where(players_table.account_id == account_id));

		printf("session_id: %s %lli\n", session_id.data(), result);

		return session_id;
	}

	std::string generate_crypto_key(const std::uint64_t account_id)
	{
		const auto data = utils::cryptography::random::get_data(16);
		const auto crypto_key = utils::cryptography::base64::encode(data);

		const auto result = database::get()->operator()(
			sqlpp::update(players_table)
				.set(players_table.crypto_key = crypto_key)
					.where(players_table.account_id == account_id));

		printf("crypto_key: %s %lli\n", crypto_key.data(), result);

		return crypto_key;
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
