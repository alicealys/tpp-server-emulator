#pragma once

#include "../database.hpp"

namespace database::players
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(account_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(session_id, sqlpp::text);
	DEFINE_FIELD(login_password, sqlpp::text);
	DEFINE_FIELD(crypto_key, sqlpp::text);
	DEFINE_FIELD(last_update, sqlpp::time_point);
	DEFINE_FIELD(creation_time, sqlpp::time_point);
	DEFINE_TABLE(players, id_t, account_id_t, session_id_t, login_password_t, crypto_key_t, last_update_t, creation_time_t);

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
			//this->last_update_ = row.last_update;
			//this->creation_time_ = row.creation_time;
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

	private:
		std::uint64_t id_;
		std::uint64_t account_id_;
		std::string session_id_;
		std::string login_password_;
		std::string crypto_key_;
		std::chrono::high_resolution_clock::time_point last_update_;
		std::chrono::high_resolution_clock::time_point creation_time_;

	};

	std::optional<player> find(const std::uint64_t id);
	std::optional<player> find_from_account(const std::uint64_t id);
	std::optional<player> find_by_session_id(const std::string session_id);

	player find_or_insert(const std::uint64_t account_id);

	std::string generate_login_password(const std::uint64_t account_id);
	std::string generate_session_id(const std::uint64_t account_id);
	std::string generate_crypto_key(const std::uint64_t account_id);

	bool update_session(const std::uint64_t player_id);
}
