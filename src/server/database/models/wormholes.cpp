#include <std_include.hpp>

#include "wormholes.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>
#include <utils/nt.hpp>

#define TABLE_DEF R"(
create table if not exists `wormholes`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	to_player_id	    bigint unsigned	not null,
	retaliate_score	    int unsigned	not null	default 0,
	flag				int unsigned	not null	default 0,
	is_open				boolean			not null	default false,
	create_date			datetime		not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`to_player_id`) references players(`id`)
))"

namespace database::wormholes
{
	auto wormholes_table = wormholes_table_t();

	wormhole_flag get_flag_id(const std::string& flag)
	{
		static std::unordered_map<std::string, wormhole_flag> flag_map =
		{
			{"BLACK", wormhole_flag_black},
			{"FRIENDLY", wormhole_flag_friendly},
		};

		const auto iter = flag_map.find(flag);
		if (iter == flag_map.end())
		{
			return wormhole_flag_invalid;
		}

		return iter->second;
	}

	void add_wormhole(const std::uint64_t player_id, const std::uint64_t to_player_id,
		const wormhole_flag flag, const std::uint32_t retaliate_point)
	{
		database::access([&](database_t& db)
		{
			db->operator()(
				sqlpp::insert_into(wormholes_table)
					.set(wormholes_table.player_id = player_id,
						 wormholes_table.to_player_id = to_player_id,
						 wormholes_table.flag = static_cast<std::uint32_t>(flag),
						 wormholes_table.retaliate_score = retaliate_point,
						 wormholes_table.create_date = std::chrono::system_clock::now())
				);
		});
	}

	std::vector<wormhole> find_active_wormholes(const std::uint64_t player_id)
	{
		return {};
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

REGISTER_TABLE(database::wormholes::table, -1)
