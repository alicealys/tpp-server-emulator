#include <std_include.hpp>

#include "players.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

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
			return (now - player.get_last_update()) > database::vars.session_timeout;
		}

#define IS_ACTIVE_EXPR player::table.last_update >= std::chrono::system_clock::now() - database::vars.session_timeout
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

	bool is_sneak_in_game(const sneak_info& info)
	{
		if (info.get_status() < status_in_game)
		{
			return false;
		}

		return true;
	}

	namespace impl
	{
		template <database_type_t Type>
		std::optional<player> find(const std::uint64_t id)
		{
			return database::access<std::optional<player>>([&](database::database_t& db)
				-> std::optional<player>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where(player::table.id == id));

				if (results.empty())
				{
					return {};
				}

				const auto& row = results.front();
				return {player(row)};
			});
		}

		template <database_type_t Type>
		bool exists(const std::uint64_t id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::count(1))
							.from(player::table)
								.where(player::table.id == id));

				return results.front().count.value() > 0;
			});
		}

		template <database_type_t Type>
		std::optional<player> find_from_account(const std::uint64_t id)
		{
			return database::access<std::optional<player>>([&](database::database_t& db)
				-> std::optional<player>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where(player::table.account_id == id));

				if (results.empty())
				{
					return {};
				}

				const auto& row = results.front();
				return {player(row)};
			});
		}

		template <database_type_t Type>
		std::optional<player> find_by_session_id(const std::string session_id, bool use_timeout, bool* is_expired)
		{
			return database::access<std::optional<player>>([&](database::database_t& db)
				-> std::optional<player>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where(player::table.session_id == session_id));

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

		template <database_type_t Type>
		player find_or_insert(const std::uint64_t account_id, bool is_real_player)
		{
			{
				const auto found = find_from_account<Type>(account_id);
				if (found.has_value())
				{
					return found.value();
				}
			}

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::insert_into(player::table)
						.set(player::table.account_id = account_id,
							 player::table.is_real_player = is_real_player,
							 player::table.currency = "EUR",
							 player::table.smart_device_id = generate_data(80, true),
							 player::table.last_update = std::chrono::system_clock::now(),
							 player::table.creation_time = std::chrono::system_clock::now()));
			});

			const auto found = find_from_account<Type>(account_id);
			if (!found.has_value())
			{
				throw std::runtime_error("[database::clients::insert] Insertion failed");
			}

			return found.value();
		}

		template <database_type_t Type>
		std::string generate_login_password(const std::uint64_t account_id)
		{
			const auto password = generate_data(16, false);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.login_password = password)
							.where(player::table.account_id == account_id));
			});

			return password;
		}

		template <database_type_t Type>
		std::string generate_session_id(const std::uint64_t account_id)
		{
			const auto session_id = generate_data(16, false);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.session_id = session_id,
							 player::table.last_update = std::chrono::system_clock::now())
								.where(player::table.account_id == account_id));
			});

			return session_id;
		}

		template <database_type_t Type>
		std::string generate_crypto_key(const std::uint64_t account_id)
		{
			const auto crypto_key = generate_data(16, true);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.crypto_key = crypto_key)
							.where(player::table.account_id == account_id));
			});

			return crypto_key;
		}

		template <database_type_t Type>
		bool update_session(const player& player)
		{
			size_t result = 0;
			if (!is_session_expired(player))
			{
				database::access([&](database::database_t& db)
				{
					result = db.get_database<Type>()->operator()(
						sqlpp::update(player::table)
							.set(player::table.last_update = std::chrono::system_clock::now())
								.where(player::table.id == player.get_id()));
				});
			}
			else
			{
				database::access([&](database::database_t& db)
				{
					result = db.get_database<Type>()->operator()(
						sqlpp::update(player::table)
							.set(player::table.last_update = std::chrono::system_clock::now(),
								 player::table.current_sneak_fob = 0,
								 player::table.current_sneak_player = 0,
								 player::table.current_sneak_platform = 0,
								 player::table.current_sneak_status = 0,
								 player::table.current_sneak_mode = 0)
									.where(player::table.id == player.get_id()));
				});
			}

			return result != 0;
		}

		template <database_type_t Type>
		void set_ip_and_port(const std::uint64_t player_id, const std::string& ex_ip, const std::uint16_t ex_port,
			const std::string& in_ip, const std::uint16_t in_port, const std::string& nat_type)
		{
			const auto nat_type_id = get_nat_type_id(nat_type);

			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.ex_ip = ex_ip,
							 player::table.in_ip = in_ip,
							 player::table.ex_port = ex_port,
							 player::table.in_port = in_port,
							 player::table.nat = nat_type_id)
								.where(player::table.id == player_id));
			});
		}

		template <database_type_t Type>
		std::vector<player> get_player_list(const std::uint32_t limit)
		{
			return database::access<std::vector<player>>([&](database::database_t& db)
				-> std::vector<player>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.unconditionally().limit(limit));

				std::vector<player> list;

				for (auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		bool try_acquire_lock(const std::uint64_t from_id, const std::uint64_t player_id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.current_lock = from_id)
							.where(player::table.id == player_id &&
								(player::table.current_lock == 0 || player::table.current_lock == from_id))
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		void release_lock(const std::uint64_t from_id, const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.current_lock = 0)
							.where(player::table.id == player_id && player::table.current_lock == from_id)
					);
			});
		}

		template <database_type_t Type>
		void abort_mother_base(const std::uint64_t player_id)
		{
			database::access([&](database::database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.current_sneak_fob = 0,
							 player::table.current_sneak_player = 0,
							 player::table.current_sneak_platform = 0,
							 player::table.current_sneak_status = 0,
							 player::table.current_sneak_mode = 0,
							 player::table.current_sneak_is_sneak = false)
								.where(player::table.id == player_id)
					);
			});
		}

		template <database_type_t Type>
		std::optional<sneak_info> find_active_sneak(const std::uint64_t owner_id, const std::uint32_t mode,
			const std::uint32_t alt_mode, bool is_sneak, bool in_game_only)
		{
			return database::access<std::optional<sneak_info>>([&](database::database_t& db)
				-> std::optional<sneak_info>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where(((player::table.current_sneak_mode == mode || 
									 player::table.current_sneak_mode == alt_mode) &&
									 player::table.current_sneak_is_sneak == is_sneak && 
									 player::table.current_sneak_player == owner_id) && IS_ACTIVE_EXPR)
					);
		
				if (results.empty())
				{
					return {};
				}

				sneak_info info(results.front());

				if (in_game_only && !is_sneak_in_game(info))
				{
					return {};
				}

				return {info};
			});
		}
	
		template <database_type_t Type>
		std::optional<sneak_info> find_active_sneak_any(const std::uint64_t owner_id, bool is_sneak, bool in_game_only)
		{
			return database::access<std::optional<sneak_info>>([&](database::database_t& db)
				-> std::optional<sneak_info>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where((player::table.current_sneak_player == owner_id &&
									   player::table.current_sneak_is_sneak == is_sneak) && IS_ACTIVE_EXPR)
					);
		
				if (results.empty())
				{
					return {};
				}

				sneak_info info(results.front());

				if (in_game_only && !is_sneak_in_game(info))
				{
					return {};
				}

				return {info};
			});
		}

		template <database_type_t Type>
		std::optional<sneak_info> get_active_sneak(const std::uint64_t fob_id)
		{
			return database::access<std::optional<sneak_info>>([&](database::database_t& db)
				-> std::optional<sneak_info>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where((player::table.current_sneak_fob == fob_id && 
									   player::table.current_sneak_is_sneak == true) &&
									   IS_ACTIVE_EXPR)
					);
		
				if (results.empty())
				{
					return {};
				}

				sneak_info info(results.front());

				if (!is_sneak_in_game(info))
				{
					return {};
				}

				return {info};
			});
		}

		template <database_type_t Type>
		bool set_active_sneak(const std::uint64_t player_id, const std::uint64_t fob_id, const std::uint64_t owner_id,
			const std::uint32_t platform, const std::uint32_t mode, const std::uint32_t status, bool is_sneak, bool is_security_challenge)
		{
			if (mode == mode_invalid)
			{
				return false;
			}

			const auto _0 = gsl::finally([=]
			{
				release_lock<Type>(player_id, owner_id);
			});

			if (!try_acquire_lock<Type>(player_id, owner_id))
			{
				return false;
			}

			const auto alt_mode = get_alt_sneak_mode(static_cast<sneak_mode>(mode));
			const auto active_sneak = find_active_sneak(owner_id, mode, alt_mode, is_sneak, false);

			if (active_sneak.has_value())
			{
				if (status < static_cast<std::uint32_t>(active_sneak->get_status()) || active_sneak->get_player_id() != player_id)
				{
					return false;
				}
			}
			else
			{
				if (status != status_menu)
				{
					return false;
				}
			}

			database::access([&](database::database_t& db)
			{
				if (status >= status_in_game)
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(player::table)
							.set(player::table.current_sneak_fob = fob_id,
								 player::table.current_sneak_player = owner_id,
								 player::table.current_sneak_platform = platform,
								 player::table.current_sneak_status = status,
								 player::table.current_sneak_mode = mode,
								 player::table.current_sneak_is_sneak = is_sneak,
								 player::table.current_sneak_security_challenge = active_sneak->is_security_challenge(),
								 player::table.current_sneak_start = std::chrono::system_clock::now())
									.where(player::table.id == player_id)
						);
				}
				else
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(player::table)
							.set(player::table.current_sneak_fob = fob_id,
								 player::table.current_sneak_player = owner_id,
								 player::table.current_sneak_platform = platform,
								 player::table.current_sneak_status = status,
								 player::table.current_sneak_mode = mode,
								 player::table.current_sneak_security_challenge = is_security_challenge,
								 player::table.current_sneak_is_sneak = is_sneak)
									.where(player::table.id == player_id)
						);
				}
			});

			return true;
		}

		template <database_type_t Type>
		std::optional<sneak_info> find_active_sneak_from_player(const std::uint64_t player_id)
		{
			return database::access<std::optional<sneak_info>>([&](database::database_t& db)
				-> std::optional<sneak_info>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where((player::table.id == player_id &&
									   player::table.current_sneak_status >= static_cast<int>(status_pre_game)) && IS_ACTIVE_EXPR)
					);
		
				if (results.empty())
				{
					return {};
				}

				return {sneak_info(results.front())};
			});
		}

		template <database_type_t Type>
		bool set_security_challenge(const std::uint64_t player_id, bool enabled)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(player::table)
						.set(player::table.security_challenge = enabled)
								.where(player::table.id == player_id)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		std::vector<player> find_with_security_challenge(const std::uint32_t limit)
		{
			return database::access<std::vector<player>>([&](database::database_t& db)
				-> std::vector<player>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(player::table))
							.from(player::table)
								.where((player::table.security_challenge == true))
					);

				std::vector<player> list;

				for (auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}
	
		template <database_type_t Type>
		std::uint64_t get_player_count()
		{
			return database::access<std::uint64_t>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::count(1))
							.from(player::table).unconditionally());

				return results.front().count.value();
			});
		}

		template <database_type_t Type>
		std::uint64_t get_online_player_count(const std::chrono::milliseconds within)
		{
			return database::access<std::uint64_t>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::count(1))
							.from(player::table)
								.where(player::table.last_update >= std::chrono::system_clock::now() - within));

				return results.front().count.value();
			});
		}
	}

	std::optional<player> find(const std::uint64_t id)
	{
		RUN_IMPL(impl::find, id);
	}

	bool exists(const std::uint64_t id)
	{
		RUN_IMPL(impl::exists, id);
	}

	std::optional<player> find_from_account(const std::uint64_t id)
	{
		RUN_IMPL(impl::find_from_account, id);
	}

	std::optional<player> find_by_session_id(const std::string session_id, bool use_timeout, bool* is_expired)
	{
		RUN_IMPL(impl::find_by_session_id, session_id, use_timeout, is_expired);
	}

	player find_or_insert(const std::uint64_t account_id, bool is_real_player)
	{
		RUN_IMPL(impl::find_or_insert, account_id, is_real_player);
	}

	std::string generate_login_password(const std::uint64_t account_id)
	{
		RUN_IMPL(impl::generate_login_password, account_id);
	}

	std::string generate_session_id(const std::uint64_t account_id)
	{
		RUN_IMPL(impl::generate_session_id, account_id);
	}

	std::string generate_crypto_key(const std::uint64_t account_id)
	{
		RUN_IMPL(impl::generate_crypto_key, account_id);
	}

	bool update_session(const player& player)
	{
		RUN_IMPL(impl::update_session, player);
	}

	void set_ip_and_port(const std::uint64_t player_id, const std::string& ex_ip, const std::uint16_t ex_port,
		const std::string& in_ip, const std::uint16_t in_port, const std::string& nat_type)
	{
		RUN_IMPL(impl::set_ip_and_port, player_id, ex_ip, ex_port, in_ip, in_port, nat_type);
	}

	std::vector<player> get_player_list(const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_player_list, limit);
	}

	void abort_mother_base(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::abort_mother_base, player_id)
	}

	bool set_active_sneak(const std::uint64_t player_id, const std::uint64_t fob_id, const std::uint64_t owner_id,
		const std::uint32_t platform, const std::uint32_t mode, const std::uint32_t status, bool is_sneak,
		bool is_security_challenge)
	{
		RUN_IMPL(impl::set_active_sneak, player_id, fob_id, owner_id, platform, mode, status, is_sneak, is_security_challenge);
	}

	std::optional<sneak_info> find_active_sneak(const std::uint64_t owner_id, const std::uint32_t mode,
		const std::uint32_t alt_mode, bool is_sneak, bool in_game_only)
	{
		RUN_IMPL(impl::find_active_sneak, owner_id, mode, alt_mode, is_sneak, in_game_only);
	}

	std::optional<sneak_info> find_active_sneak(const std::uint64_t owner_id, bool is_sneak, bool in_game_only)
	{
		RUN_IMPL(impl::find_active_sneak_any, owner_id, is_sneak, in_game_only);
	}

	std::optional<sneak_info> find_active_sneak_from_player(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::find_active_sneak_from_player, player_id);
	}

	std::optional<sneak_info> get_active_sneak(const std::uint64_t fob_id)
	{
		RUN_IMPL(impl::get_active_sneak, fob_id);
	}

	bool set_security_challenge(const std::uint64_t player_id, bool enabled)
	{
		RUN_IMPL(impl::set_security_challenge, player_id, enabled);
	}

	std::vector<player> find_with_security_challenge(const std::uint32_t limit)
	{
		RUN_IMPL(impl::find_with_security_challenge, limit);
	}

	std::uint64_t get_player_count()
	{
		RUN_IMPL(impl::get_player_count);
	}

	std::uint64_t get_online_player_count(const std::chrono::milliseconds within)
	{
		RUN_IMPL(impl::get_online_player_count, within);
	}

	std::uint64_t get_online_player_count()
	{
		return get_online_player_count(database::vars.session_timeout);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.players.create");
		}
	};
}

REGISTER_TABLE(database::players::table, 1)
