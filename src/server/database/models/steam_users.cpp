#include <std_include.hpp>

#include "steam_users.hpp"
#include "combat_deployments.hpp"
#include "event_rankings.hpp"
#include "fobs.hpp"
#include "fob_events.hpp"
#include "items.hpp"
#include "mgo_characters.hpp"
#include "mgo_color_purchases.hpp"
#include "mgo_data.hpp"
#include "mgo_item_purchases.hpp"
#include "mgo_stats.hpp"
#include "mgo_titles.hpp"
#include "pf_league.hpp"
#include "players.hpp"
#include "player_data.hpp"
#include "player_follows.hpp"
#include "player_records.hpp"
#include "shop_purchases.hpp"
#include "sneak_results.hpp"
#include "steam_users.hpp"
#include "variables.hpp"
#include "wormholes.hpp"

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

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t account_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(steam_user::table)
						.where(steam_user::table.account_id == account_id));
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

	void delete_player_data(const std::uint64_t account_id)
	{
		RUN_IMPL(impl::delete_player_data, account_id);
	}

	bool delete_all_user_data(const std::uint64_t account_id)
	{
		steam_users::delete_player_data(account_id);

		const auto player = players::find_from_account(account_id);
		if (!player.has_value())
		{
			return false;
		}

		pf_league::delete_player_data(player->get_id());

		sneak_results::delete_player_data(player->get_id());
		wormholes::delete_player_data(player->get_id());
		fobs::delete_player_data(player->get_id());

		player_follows::delete_player_data(player->get_id());
		player_records::delete_player_data(player->get_id());
		player_data::delete_player_data(player->get_id());

		shop_purchases::delete_player_data(player->get_id());
		mgo_item_purchases::delete_player_data(player->get_id());
		mgo_color_purchases::delete_player_data(player->get_id());

		mgo_titles::delete_player_data(player->get_id());
		mgo_stats::delete_player_data(player->get_id());
		mgo_characters::delete_player_data(player->get_id());
		mgo_data::delete_player_data(player->get_id());

		combat_deployments::delete_player_data(player->get_id());
		event_rankings::delete_player_data(player->get_id());
		items::delete_player_data(player->get_id());

		players::delete_player_data(player->get_id());

		return true;
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
