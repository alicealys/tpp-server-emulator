#include <std_include.hpp>

#include "players.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#define TABLE_DEF R"(
create table if not exists `players`
(
	id						bigint unsigned	not null	auto_increment,
	account_id				bigint unsigned	not null,
	session_id				char(32)		default null,
	login_password			char(32)		default null,
	last_update				datetime		default null,
	crypto_key				char(32)		default null,
	smart_device_id			char(128)		default null,
	currency				varchar(32)		default null,
	ex_ip					varchar(15)		default null,
	in_ip					varchar(15)		default null,
	ex_port					int unsigned	default 0,
	in_port					int unsigned	default 0,
	nat						int unsigned	default 0,
	creation_time			datetime        not null,
	current_lock			int unsigned	not null		default 0,
	current_sneak_mode		int unsigned	not null		default 0,
	current_sneak_fob		bigint unsigned not null		default 0,
	current_sneak_player	bigint unsigned not null		default 0,
	current_sneak_platform	int unsigned	not null		default 0,
	current_sneak_status	int unsigned	not null		default 0,
	current_sneak_start 	datetime,
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

		std::unordered_map<std::string, sneak_mode> sneak_mode_map =
		{
			{"VISIT", mode_visit},
			{"SHAM", mode_sham},
			{"ACTUAL", mode_actual},
		};

		bool is_session_expired(const player& player)
		{
			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());
			return (now - player.get_last_update()) > session_timeout;
		}
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

	sneak_mode get_sneak_mode_id(const std::string& mode)
	{
		const auto iter = sneak_mode_map.find(mode);
		if (iter == sneak_mode_map.end())
		{
			return mode_invalid;
		}

		return iter->second;
	}

	sneak_mode get_alt_sneak_mode(const sneak_mode mode)
	{
		auto alt_mode = database::players::mode_invalid;

		if (mode == database::players::mode_actual)
		{
			alt_mode = database::players::mode_visit;
		}
		else if (mode == database::players::mode_visit)
		{
			alt_mode = database::players::mode_actual;
		}

		return alt_mode;
	}

	players_table_t players_table;

	std::optional<player> find(const std::uint64_t id)
	{
		return database::access<std::optional<player>>([&](database::database_t& db)
			-> std::optional<player>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(players::players_table))
						.from(players::players_table)
							.where(players::players_table.id == id));

			if (results.empty())
			{
				return {};
			}

			const auto& row = results.front();
			return {player(row)};
		});
	}

	std::optional<player> find_from_account(const std::uint64_t id)
	{
		return database::access<std::optional<player>>([&](database::database_t& db)
			-> std::optional<player>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(players::players_table))
						.from(players::players_table)
							.where(players::players_table.account_id == id));

			if (results.empty())
			{
				return {};
			}

			const auto& row = results.front();
			return {player(row)};
		});
	}

	std::optional<player> find_by_session_id(const std::string session_id, bool use_timeout, bool* is_expired)
	{
		return database::access<std::optional<player>>([&](database::database_t& db)
			-> std::optional<player>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(players::players_table))
						.from(players::players_table)
							.where(players::players_table.session_id == session_id));

			if (results.empty())
			{
				return {};
			}

			const auto& row = results.front();
			player player(row);

			const auto expired = is_session_expired(player);
			if (is_expired != nullptr)
			{
				*is_expired = expired;
			}

			if (use_timeout && expired)
			{
				return {};
			}

			return {player};
		});
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

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::insert_into(players::players_table)
					.set(players::players_table.account_id = account_id,
						 players::players_table.currency = "EUR",
						 players::players_table.smart_device_id = generate_data(80, true),
						 players::players_table.last_update = std::chrono::system_clock::now(),
						 players::players_table.creation_time = std::chrono::system_clock::now()));
		});

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

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players::players_table)
					.set(players::players_table.login_password = password)
						.where(players::players_table.account_id == account_id));
		});

		return password;
	}

	std::string generate_session_id(const std::uint64_t account_id)
	{
		const auto session_id = generate_data(16, false);

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players::players_table)
					.set(players::players_table.session_id = session_id,
						 players::players_table.last_update = std::chrono::system_clock::now())
							.where(players::players_table.account_id == account_id));
		});

		return session_id;
	}

	std::string generate_crypto_key(const std::uint64_t account_id)
	{
		const auto crypto_key = generate_data(16, true);

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players::players_table)
					.set(players::players_table.crypto_key = crypto_key)
						.where(players::players_table.account_id == account_id));
		});

		return crypto_key;
	}

	bool update_session(const player& player)
	{
		size_t result = 0;
		if (!is_session_expired(player))
		{
			database::access([&](database::database_t& db)
			{
				result = db->operator()(
					sqlpp::update(players::players_table)
						.set(players::players_table.last_update = std::chrono::system_clock::now())
							.where(players::players_table.id == player.get_id()));
			});
		}
		else
		{
			database::access([&](database::database_t& db)
			{
				result = db->operator()(
					sqlpp::update(players::players_table)
						.set(players::players_table.last_update = std::chrono::system_clock::now(),
							 players::players_table.current_sneak_fob = 0,
							 players::players_table.current_sneak_player = 0,
							 players::players_table.current_sneak_platform = 0,
							 players::players_table.current_sneak_status = 0,
							 players::players_table.current_sneak_mode = 0)
								.where(players::players_table.id == player.get_id()));
			});
		}

		return result != 0;
	}

	void set_ip_and_port(const std::uint64_t player_id, const std::string& ex_ip, const std::uint16_t ex_port,
		const std::string& in_ip, const std::uint16_t in_port, const std::string& nat_type)
	{
		const auto nat_type_id = get_nat_type_id(nat_type);

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players::players_table)
					.set(players::players_table.ex_ip = ex_ip,
						 players::players_table.in_ip = in_ip,
						 players::players_table.ex_port = ex_port,
						 players::players_table.in_port = in_port,
						 players::players_table.nat = nat_type_id)
							.where(players::players_table.id == player_id));
		});
	}

	std::vector<player> get_player_list(const std::uint32_t limit)
	{
		return database::access<std::vector<player>>([&](database::database_t& db)
			-> std::vector<player>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(players::players_table))
						.from(players::players_table)
							.unconditionally().limit(limit));

			std::vector<player> list;

			for (auto& row : results)
			{
				list.emplace_back(row);
			}

			return list;
		});
	}

	bool try_acquire_lock(const std::uint64_t from_id, const std::uint64_t player_id)
	{
		return database::access<bool>([&](database::database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::update(players::players_table)
					.set(players::players_table.current_lock = from_id)
						.where(players::players_table.id == player_id &&
							(players::players_table.current_lock == 0 || players::players_table.current_lock == from_id))
				);

			return result != 0;
		});
	}

	void release_lock(const std::uint64_t from_id, const std::uint64_t player_id)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players::players_table)
					.set(players_table.current_lock = 0)
						.where(players_table.id == player_id && players_table.current_lock == from_id)
				);
		});
	}

	void abort_mother_base(const std::uint64_t player_id)
	{
		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players_table)
					.set(players_table.current_sneak_fob = 0,
						 players_table.current_sneak_player = 0,
						 players_table.current_sneak_platform = 0,
						 players_table.current_sneak_status = 0,
						 players_table.current_sneak_mode = 0)
							.where(players_table.id == player_id)
				);
		});
	}

	std::optional<sneak_info> find_active_sneak(const std::uint64_t owner_id, const std::uint32_t mode, 
		const std::uint32_t alt_mode)
	{
		return database::access<std::optional<sneak_info>>([&](database::database_t& db)
			-> std::optional<sneak_info>
		{
			auto results = db->operator()(
				sqlpp::select(
					sqlpp::all_of(players_table))
						.from(players_table)
							.where((players_table.current_sneak_mode == mode || 
								 players_table.current_sneak_mode == alt_mode) &&
								 players_table.current_sneak_player == owner_id)
				);
		
			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			if (results.empty())
			{
				return {};
			}

			for (auto& row : results)
			{
				if ((now - row.last_update.value().time_since_epoch()) <= session_timeout)
				{
					return {sneak_info(row)};
				}
			}

			return {};
		});
	}
	
	bool set_active_sneak(const std::uint64_t player_id, const std::uint64_t fob_id, const std::uint64_t owner_id,
		const std::uint32_t platform, const std::uint32_t mode, const std::uint32_t status)
	{
		if (mode == mode_invalid)
		{
			return false;
		}

		const auto _0 = gsl::finally([=]
		{
			release_lock(player_id, owner_id);
		});

		if (!try_acquire_lock(player_id, owner_id))
		{
			return false;
		}

		const auto alt_mode = get_alt_sneak_mode(static_cast<sneak_mode>(mode));
		const auto active_sneak = find_active_sneak(owner_id, mode, alt_mode);
		
		if (active_sneak.has_value())
		{
			return false;
		}

		database::access([&](database::database_t& db)
		{
			db->operator()(
				sqlpp::update(players_table)
					.set(players_table.current_sneak_fob = fob_id,
						 players_table.current_sneak_player = owner_id,
						 players_table.current_sneak_platform = platform,
						 players_table.current_sneak_status = status,
						 players_table.current_sneak_mode = mode)
							.where(players_table.id == player_id)
				);
		});

		return true;
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
