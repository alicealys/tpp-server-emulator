#include <std_include.hpp>

#include "player_follows.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

namespace database::player_follows
{
	namespace impl
	{
		template <database_type_t Type>
		bool add_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
		{
			if (player_id == to_player_id)
			{
				return false;
			}

			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(player_follow::table)
						.set(player_follow::table.player_id = player_id,
							 player_follow::table.to_player_id = to_player_id)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool remove_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::remove_from(player_follow::table)
						.where(player_follow::table.player_id == player_id &&
							 player_follow::table.to_player_id == to_player_id)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		std::unordered_set<std::uint64_t> get_follows(const std::uint64_t player_id)
		{
			return database::access<std::unordered_set<std::uint64_t>>([&](database_t& db)
				-> std::unordered_set<std::uint64_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_follow::table.to_player_id)
							.from(player_follow::table)
								.where(player_follow::table.player_id == player_id));

				std::unordered_set<std::uint64_t> list;

				for (auto& row : results)
				{
					list.insert(row.to_player_id);
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::unordered_set<std::uint64_t> get_followers(const std::uint64_t to_player_id)
		{
			return database::access<std::unordered_set<std::uint64_t>>([&](database_t& db)
				-> std::unordered_set<std::uint64_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(player_follow::table.player_id)
							.from(player_follow::table)
								.where(player_follow::table.to_player_id == to_player_id));

				std::unordered_set<std::uint64_t> list;

				for (auto& row : results)
				{
					list.insert(row.player_id);
				}

				return list;
			});
		}
	}

	bool add_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::add_follow, player_id, to_player_id);
	}

	bool remove_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::remove_follow, player_id, to_player_id);
	}

	std::unordered_set<std::uint64_t> get_follows(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_follows, player_id);
	}

	std::unordered_set<std::uint64_t> get_followers(const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::get_followers, to_player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.player_follows.create");
		}
	};
}

REGISTER_TABLE(database::player_follows::table, -1)
