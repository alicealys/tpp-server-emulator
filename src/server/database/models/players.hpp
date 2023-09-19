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
	DEFINE_TABLE(players, id_field_t, account_id_field_t, session_id_field_t, 
		login_password_field_t, crypto_key_field_t, smart_device_id_field_t, 
		currency_field_t,last_update_field_t, ex_ip_field_t, ex_port_field_t, 
		in_ip_field_t, in_port_field_t, nat_field_t, creation_time_field_t);

	std::uint32_t get_nat_type_id(const std::string& nat_type);
	std::string get_nat_type(const std::uint32_t nat_type_id);

	constexpr auto session_heartbeat = 60s;
	constexpr auto session_timeout = 200s;

	class player
	{
	public:
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

		std::chrono::microseconds get_last_update()
		{
			return this->last_update_;
		}

		std::chrono::microseconds get_creation_time()
		{
			return this->creation_time_;
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

	};

	std::optional<player> find(const std::uint64_t id);
	std::optional<player> find_from_account(const std::uint64_t id);
	std::optional<player> find_by_session_id(const std::string session_id, bool use_timeout = true, bool* is_expired = nullptr);

	player find_or_insert(const std::uint64_t account_id);

	std::string generate_login_password(const std::uint64_t account_id);
	std::string generate_session_id(const std::uint64_t account_id);
	std::string generate_crypto_key(const std::uint64_t account_id);

	bool update_session(const std::uint64_t player_id);

	void set_ip_and_port(const std::uint64_t player_id, const std::string& ex_ip, const std::uint16_t ex_port,
		const std::string& in_ip, const std::uint16_t in_port, const std::string& nat_type);

	std::vector<player> get_player_list(const std::uint32_t limit = 10);
}
