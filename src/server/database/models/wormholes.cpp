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
		const wormhole_flag flag, const bool is_open, const std::uint32_t retaliate_point)
	{
		database::access([&](database_t& db)
		{
			db->operator()(
				sqlpp::insert_into(wormholes_table)
					.set(wormholes_table.player_id = player_id,
						 wormholes_table.to_player_id = to_player_id,
						 wormholes_table.flag = static_cast<std::uint32_t>(flag),
						 wormholes_table.retaliate_score = retaliate_point,
						 wormholes_table.is_open = is_open,
						 wormholes_table.create_date = std::chrono::system_clock::now())
				);
		});
	}

	std::vector<wormhole> find_active_wormholes(const std::uint64_t player_id, const std::uint64_t other_player_id)
	{
		return database::access<std::vector<wormhole>>([&](database_t& db)
		{
			auto results = db->operator()(
				sqlpp::select(sqlpp::all_of(wormholes_table))
					.from(wormholes_table)
						.where(wormholes_table.is_open == true && wormholes_table.retaliate_score > 0 &&
							   ((wormholes_table.player_id == player_id && wormholes_table.to_player_id == other_player_id) ||
								(wormholes_table.player_id == other_player_id && wormholes_table.to_player_id == player_id)))
				);

			std::vector<wormhole> list;

			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			for (auto& row : results)
			{
				wormhole w(row);

				if (now - w.get_create_date() > 24h * 30)
				{
					continue;
				}

				list.emplace_back(w);
			}

			return list;
		});
	}

	std::unordered_map<std::uint64_t, wormhole_status> find_active_wormholes(const std::uint64_t player_id)
	{
		return database::access<std::unordered_map<std::uint64_t, wormhole_status>>([&](database_t& db)
		{
			auto results = db->operator()(
				sqlpp::select(sqlpp::all_of(wormholes_table))
					.from(wormholes_table)
						.where(wormholes_table.is_open == true && wormholes_table.retaliate_score > 0 &&
							   wormholes_table.player_id == player_id || wormholes_table.to_player_id == player_id)
				);

			std::unordered_map<std::uint64_t, wormhole_status> list;

			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			for (auto& row : results)
			{
				wormhole w(row);

				const auto to_player_id = player_id == w.get_to_player_id()
					? w.get_player_id() : w.get_to_player_id();

				auto& status = list[to_player_id];

				status.player_id = player_id;
				status.to_player_id = to_player_id;

				if (now - w.get_create_date() > wormhole_duration)
				{
					continue;
				}

				if (w.get_to_player_id() == to_player_id)
				{
					status.score += w.get_retaliate_score();
				}
				else
				{
					status.first = false;
					status.score -= w.get_retaliate_score();
				}

				if (w.get_create_date() > status.expire)
				{
					status.expire = w.get_create_date();
				}
			}

			for (auto i = list.begin(), end = list.end(); i != end; )
			{
				if (i->second.score > 0)
				{
					i->second.open = true;
					i->second.expire += wormhole_duration;
					++i;
				}
				else
				{
					i = list.erase(i);
				}
			}

			return list;
		});
	}

	wormhole_status get_wormhole_status(const std::uint64_t from_player_id, const std::uint64_t to_player_id)
	{
		const auto wormholes = find_active_wormholes(from_player_id, to_player_id);

		wormhole_status status{};
		status.first = true;

		for (auto& wormhole : wormholes)
		{
			if (wormhole.get_to_player_id() == to_player_id)
			{
				status.score += wormhole.get_retaliate_score();
			}
			else
			{
				status.first = false;
				status.score -= wormhole.get_retaliate_score();
			}

			if (wormhole.get_create_date() > status.expire)
			{
				status.expire = wormhole.get_create_date();
			}
		}

		status.expire += wormhole_duration;
		status.open = status.score > 0;

		return status;
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
