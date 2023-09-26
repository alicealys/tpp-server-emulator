#pragma once

#include "../database.hpp"

namespace database::players
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(account_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(session_id, sqlpp::text);
	DEFINE_FIELD(login_password, sqlpp::text);
	DEFINE_FIELD(crypto_key, sqlpp::text);
	DEFINE_FIELD(smart_device_id, sqlpp::text);
	DEFINE_FIELD(currency, sqlpp::text);
	DEFINE_FIELD(last_update, sqlpp::time_point);
	DEFINE_FIELD(ex_ip, sqlpp::text);
	DEFINE_FIELD(ex_port, sqlpp::integer_unsigned);
	DEFINE_FIELD(in_ip, sqlpp::text);
	DEFINE_FIELD(in_port, sqlpp::integer_unsigned);
	DEFINE_FIELD(nat, sqlpp::integer_unsigned);
	DEFINE_FIELD(creation_time, sqlpp::time_point);
	DEFINE_FIELD(security_challenge, sqlpp::boolean);
	DEFINE_FIELD(current_lock, sqlpp::integer_unsigned);
	DEFINE_FIELD(current_sneak_mode, sqlpp::integer_unsigned);
	DEFINE_FIELD(current_sneak_fob, sqlpp::integer_unsigned);
	DEFINE_FIELD(current_sneak_player, sqlpp::integer_unsigned);
	DEFINE_FIELD(current_sneak_platform, sqlpp::integer_unsigned);
	DEFINE_FIELD(current_sneak_status, sqlpp::integer_unsigned);
	DEFINE_FIELD(current_sneak_is_sneak, sqlpp::boolean);
	DEFINE_FIELD(current_sneak_start, sqlpp::time_point);
	DEFINE_TABLE(players, id_field_t, account_id_field_t, session_id_field_t, 
		login_password_field_t, crypto_key_field_t, smart_device_id_field_t, 
		currency_field_t,last_update_field_t, ex_ip_field_t, ex_port_field_t, 
		in_ip_field_t, in_port_field_t, nat_field_t, creation_time_field_t,
		current_lock_field_t,
		security_challenge_field_t,
		current_sneak_mode_field_t, current_sneak_fob_field_t, 
		current_sneak_player_field_t, current_sneak_platform_field_t,
		current_sneak_status_field_t, current_sneak_is_sneak_field_t,
		current_sneak_start_field_t);

	std::uint32_t get_nat_type_id(const std::string& nat_type);
	std::string get_nat_type(const std::uint32_t nat_type_id);

	constexpr auto session_heartbeat = 60s;
	constexpr auto session_timeout = 200s;

	extern players_table_t players_table;

	enum sneak_mode
	{
		mode_none = 0,
		mode_visit = 1,
		mode_sham = 2,
		mode_actual = 3,
		mode_invalid = 4,
		mode_count
	};

	enum sneak_status
	{
		status_none = 0,
		status_menu = 1,
		status_pre_game = 2,
		status_in_game = 3,
		status_count
	};

	sneak_mode get_sneak_mode_id(const std::string& mode);
	sneak_mode get_alt_sneak_mode(const sneak_mode mode);

	class player
	{
	public:
		player() = default;

		template <typename ...Args>
		player(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->account_id_ = row.account_id;
			this->session_id_ = row.session_id;
			this->login_password_ = row.login_password;
			this->crypto_key_ = row.crypto_key;
			this->smart_device_id_ = row.smart_device_id;
			this->currency_ = row.currency;
			this->ex_ip_ = row.ex_ip;
			this->ex_port_ = static_cast<std::uint16_t>(row.ex_port);
			this->in_ip_ = row.in_ip;
			this->in_port_ = static_cast<std::uint16_t>(row.in_port);
			this->nat_ = static_cast<std::uint32_t>(row.nat);
			this->last_update_ = row.last_update.value().time_since_epoch();
			this->creation_time_ = row.creation_time.value().time_since_epoch();
			this->security_challenge_ = row.security_challenge;
		}

		std::uint64_t get_id() const
		{
			return this->id_;
		}

		std::uint64_t get_account_id() const
		{
			return this->account_id_;
		}

		std::string get_login_password() const
		{
			return this->login_password_;
		}

		std::string get_crypto_key() const
		{
			return this->crypto_key_;
		}

		std::string get_smart_device_id() const
		{
			return this->smart_device_id_;
		}

		std::string get_currency() const
		{
			return this->currency_;
		}

		std::string get_ex_ip() const
		{
			return this->ex_ip_;
		}

		std::string get_in_ip() const
		{
			return this->in_ip_;
		}

		std::uint16_t get_ex_port() const
		{
			return this->ex_port_;
		}

		std::uint16_t get_in_port() const
		{
			return this->in_port_;
		}

		std::string get_nat() const
		{
			return get_nat_type(this->nat_);
		}

		std::chrono::microseconds get_last_update() const
		{
			return this->last_update_;
		}

		std::chrono::microseconds get_creation_time() const
		{
			return this->creation_time_;
		}

		int is_security_challenge_enabled() const
		{
			return this->security_challenge_;
		}

		std::string get_session_id() const
		{
			return this->session_id_;
		}

		void set_session_id(const std::string& session_id)
		{
			this->session_id_ = session_id;
		}

	private:
		std::uint64_t id_;
		std::uint64_t account_id_;

		std::string session_id_;
		std::string login_password_;
		std::string crypto_key_;
		std::string smart_device_id_;
		std::string currency_;

		std::string ex_ip_;
		std::string in_ip_;

		std::uint32_t nat_;

		std::uint16_t ex_port_;
		std::uint16_t in_port_;

		std::chrono::microseconds last_update_;
		std::chrono::microseconds creation_time_;

		bool security_challenge_;

	};

	class sneak_info
	{
	public:
		template <typename ...Args>
		sneak_info(const sqlpp::result_row_t<Args...>& row)
		{
			this->mode_ = static_cast<sneak_mode>(static_cast<std::uint32_t>(row.current_sneak_mode));
			this->status_ = static_cast<sneak_status>(static_cast<std::uint32_t>(row.current_sneak_status));
			this->player_id_ = row.id;
			this->fob_id_ = row.current_sneak_fob;
			this->owner_id_ = row.current_sneak_player;
			this->platform_ = static_cast<std::uint32_t>(row.current_sneak_platform);
			this->is_sneak_ = row.current_sneak_is_sneak;
			this->start_time_ = row.current_sneak_start.value().time_since_epoch();
		}

		sneak_mode get_mode() const
		{
			return this->mode_;
		}

		sneak_status get_status() const
		{
			return this->status_;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint64_t get_owner_id() const
		{
			return this->owner_id_;
		}

		std::uint64_t get_fob_id() const
		{
			return this->fob_id_;
		}

		std::uint32_t get_platform() const
		{
			return this->platform_;
		}

		bool is_sneak() const
		{
			return this->is_sneak_;
		}

		std::chrono::microseconds get_start_time() const
		{
			return this->start_time_;
		}

	private:
		sneak_mode mode_;
		sneak_status status_;
		std::uint64_t player_id_;
		std::uint64_t fob_id_;
		std::uint64_t owner_id_;
		std::uint32_t platform_;
		bool is_sneak_;
		std::chrono::microseconds start_time_;

	};

	std::optional<player> find(const std::uint64_t id);
	std::optional<player> find_from_account(const std::uint64_t id);
	std::optional<player> find_by_session_id(const std::string session_id, bool use_timeout = true, bool* is_expired = nullptr);

	player find_or_insert(const std::uint64_t account_id);

	std::string generate_login_password(const std::uint64_t account_id);
	std::string generate_session_id(const std::uint64_t account_id);
	std::string generate_crypto_key(const std::uint64_t account_id);

	bool update_session(const player& player);

	void set_ip_and_port(const std::uint64_t player_id, const std::string& ex_ip, const std::uint16_t ex_port,
		const std::string& in_ip, const std::uint16_t in_port, const std::string& nat_type);

	std::vector<player> get_player_list(const std::uint32_t limit = 10);

	void abort_mother_base(const std::uint64_t player_id);

	bool set_active_sneak(const std::uint64_t player_id, const std::uint64_t fob_id, const std::uint64_t owner_id,
		const std::uint32_t platform, const std::uint32_t mode, const std::uint32_t status, bool is_sneak);

	std::optional<sneak_info> find_active_sneak(const std::uint64_t owner_id, const std::uint32_t mode,
		const std::uint32_t alt_mode = mode_invalid, bool is_sneak = true, bool in_game_only = false);
	std::optional<sneak_info> find_active_sneak(const std::uint64_t owner_id, bool is_sneak = true, bool in_game_only = false);
	std::optional<sneak_info> find_active_sneak_from_player(const std::uint64_t player_id);
	std::optional<sneak_info> get_active_sneak(const std::uint64_t fob_id);

	bool set_security_challenge(const std::uint64_t player_id, bool enabled);

	std::vector<player> find_with_security_challenge(const std::uint32_t limit);
}
