#include <std_include.hpp>

#include "player_follows.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `player_follows`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	to_player_id	    bigint unsigned	not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`to_player_id`) references players(`id`),
	unique key `unique_player_follow` (`player_id`, `to_player_id`),
    check (`player_id` != `to_player_id`)
))"

namespace database::player_follows
{
	bool add_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		if (player_id == to_player_id)
		{
			return false;
		}

		return database::access<bool>([&](database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::insert_into(player_follow::table)
					.set(player_follow::table.player_id = player_id,
						 player_follow::table.to_player_id = to_player_id)
				);

			return result != 0;
		});
	}

	bool remove_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		return database::access<bool>([&](database_t& db)
		{
			const auto result = db->operator()(
				sqlpp::remove_from(player_follow::table)
					.where(player_follow::table.player_id == player_id &&
						 player_follow::table.to_player_id == to_player_id)
				);

			return result != 0;
		});
	}

	std::unordered_set<std::uint64_t> get_follows(const std::uint64_t player_id)
	{
		return database::access<std::unordered_set<std::uint64_t>>([&](database_t& db)
			-> std::unordered_set<std::uint64_t>
		{
			auto results = db->operator()(
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

	std::unordered_set<std::uint64_t> get_followers(const std::uint64_t to_player_id)
	{
		return database::access<std::unordered_set<std::uint64_t>>([&](database_t& db)
			-> std::unordered_set<std::uint64_t>
		{
			auto results = db->operator()(
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

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database->execute(TABLE_DEF);
		}
	};
}

REGISTER_TABLE(database::player_follows::table, -1)
