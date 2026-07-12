#include <std_include.hpp>

#include "steam_users.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::steam_users
{
	GET_FIELD_C(steam_user, std::uint64_t, id);
	GET_FIELD_C(steam_user, std::uint64_t, account_id);
	GET_FIELD_C(steam_user, std::string, auth_token);

	namespace impl
	{
		template <database_type_t Type>
		bool set_auth_token(const std::uint64_t account_id, const std::string& auth_token)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto exists = db.get_database<Type>()->operator()(
					sqlpp::select(steam_user::table.id)
						.from(steam_user::table)
							.where(steam_user::table.account_id == account_id));

				if (exists.empty())
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::insert_into(steam_user::table)
							.set(steam_user::table.account_id = account_id, 
								 steam_user::table.auth_token = auth_token, 
								 steam_user::table.expire_date = std::chrono::system_clock::now() + auth_token_duration
							));
					return results != 0u;
				}
				else
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::update(steam_user::table)
							.set(steam_user::table.auth_token = auth_token, 
								 steam_user::table.expire_date = std::chrono::system_clock::now() + auth_token_duration)
									.where(steam_user::table.account_id == account_id)
						);
					return results != 0u;
				}
			});
		}

		template <database_type_t Type>
		bool authenticate(const std::uint64_t account_id, const std::string& auth_token)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(steam_user::table.account_id)
						.from(steam_user::table)
							.where(steam_user::table.account_id == account_id &&
								   steam_user::table.auth_token == auth_token &&  
								   steam_user::table.expire_date > std::chrono::system_clock::now()
						));
				return !results.empty();
			});
		}
	}

	bool set_auth_token(const std::uint64_t account_id, const std::string& auth_token)
	{
		RUN_IMPL(impl::set_auth_token, account_id, auth_token);
	}

	bool authenticate(const std::uint64_t account_id, const std::string& auth_token)
	{
		RUN_IMPL(impl::authenticate, account_id, auth_token);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.steam_users.create");
		}
	};
}

REGISTER_TABLE(database::steam_users::table, 1)
