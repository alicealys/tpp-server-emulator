#pragma once

#include "../database.hpp"

namespace database::steam_users
{
	constexpr const auto auth_token_duration = 24h * 365;

	class steam_user
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(account_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(auth_token, sqlpp::text);
		DEFINE_FIELD(expire_date, sqlpp::time_point);
		DEFINE_TABLE(steam_users, id_field_t, account_id_field_t, auth_token_field_t, expire_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		steam_user(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->account_id_ = row.account_id;
			this->auth_token_ = row.auth_token;
			this->expire_date_ = row.expire_date;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, account_id);
		GET_FIELD_H(std::string, auth_token);
		GET_FIELD_H(std::chrono::system_clock::time_point, expire_date);
	};

	bool set_auth_token(const std::uint64_t account_id, const std::string& auth_token);
	bool authenticate(const std::uint64_t account_id, const std::string& auth_token);
	void delete_player_data(const std::uint64_t account_id);
	bool delete_all_user_data(const std::uint64_t account_id);
}
