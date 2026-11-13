#include <std_include.hpp>

#include "player_follows.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::player_follows
{
	namespace
	{
		auto get_select_query()
		{
			const auto select = sqlpp::select(sqlpp::all_of(players::player::table), 
								  player_records::player_record::table.player_id,
								  player_records::player_record::table.fob_grade,
								  player_records::player_record::table.prev_fob_grade,
								  player_records::player_record::table.fob_point,
								  player_records::player_record::table.fob_rank,
								  player_records::player_record::table.prev_fob_rank,
								  player_records::player_record::table.insurance_end,
								  player_records::player_record::table.league_grade,
								  player_records::player_record::table.prev_league_grade,
								  player_records::player_record::table.league_rank,
								  player_records::player_record::table.prev_league_rank,
								  player_records::player_record::table.league_point,
								  player_records::player_record::table.pf_point,
								  player_records::player_record::table.pf_point_add,
								  player_records::player_record::table.event_point,
								  player_records::player_record::table.playtime,
								  player_records::player_record::table.point,
								  player_records::player_record::table.fob_defense_win,
								  player_records::player_record::table.fob_defense_lose,
								  player_records::player_record::table.fob_sneak_win,
								  player_records::player_record::table.fob_sneak_lose,
								  player_records::player_record::table.shield_date,
								  player_records::player_record::table.has_fob,
								  player_records::player_record::table.daily_last,
								  player_records::player_record::table.daily_last_ack,
								  player_records::player_record::table.daily_total,
								  player_records::player_record::table.league_last_ack);

			return select;
		}
	}

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
		follow_list get_follows(const std::uint64_t player_id)
		{
			follow_list list;

			const auto joined_player_record =
				players::player::table.join(player_records::player_record::table)
				.on(players::player::table.id == player_records::player_record::table.player_id);

			const auto joined_tables = joined_player_record.join(player_follow::table)
				.on(player_follow::table.to_player_id == players::player::table.id &&
					player_records::player_record::table.player_id == player_follow::table.to_player_id);

			const auto query = get_select_query().from(joined_tables)
				.where(player_follow::table.player_id == player_id);

			database::access([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(query);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		follow_list get_followers(const std::uint64_t to_player_id)
		{
			follow_list list;

			const auto joined_player_record =
				players::player::table.join(player_records::player_record::table)
				.on(players::player::table.id == player_records::player_record::table.player_id);

			const auto joined_tables = joined_player_record.join(player_follow::table)
				.on(player_follow::table.player_id == players::player::table.id &&
					player_records::player_record::table.player_id == player_follow::table.player_id);

			const auto query = get_select_query().from(joined_tables)
				.where(player_follow::table.to_player_id == to_player_id);

			database::access([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(query);

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			return list;
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(player_follow::table)
						.where(player_follow::table.to_player_id == player_id || player_follow::table.player_id == player_id));
			});
		}
	}

	bool follow_list::contains(const std::uint64_t player_id) const
	{
		const auto iter = std::ranges::find_if(this->begin(), this->end(), [&](const player_follow& follow)
		{
			return player_id == follow.get_player_id();
		});

		return iter != this->end();
	}

	bool add_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::add_follow, player_id, to_player_id);
	}

	bool remove_follow(const std::uint64_t player_id, const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::remove_follow, player_id, to_player_id);
	}

	follow_list get_follows(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_follows, player_id);
	}

	follow_list get_followers(const std::uint64_t to_player_id)
	{
		RUN_IMPL(impl::get_followers, to_player_id);
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
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
