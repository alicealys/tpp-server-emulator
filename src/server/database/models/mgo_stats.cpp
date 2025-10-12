#include <std_include.hpp>

#include "mgo_stats.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_stats
{
	std::unordered_set<std::uint32_t>& get_rules()
	{
		static std::unordered_set<std::uint32_t> rules =
		{
			rule_bounty_hunter,
			rule_comm_control,
			rule_cloak_and_dagger,
			rule_sabotage,
		};

		return rules;
	}

	std::unordered_set<std::uint32_t>& get_general_stats()
	{
		static std::unordered_set<std::uint32_t> stats =
		{
			stat_abandon_midmatch,
			stat_amazing_throw,
			stat_ascension_neg_xp,
			stat_assist,
			stat_calc_xp,
			stat_charm,
			stat_cqc,
			stat_death,
			stat_disconnection,
			stat_friend_kill,
			stat_fulton,
			stat_fultoned,
			stat_fultonsave,
			stat_games_won,
			stat_gear_points,
			stat_headshot,
			stat_host_abandonned,
			stat_interrogation,
			stat_kill,
			stat_knife,
			stat_matches_lost,
			stat_matches_played,
			stat_matches_started,
			stat_matches_won,
			stat_raw_xp,
			stat_stunned,
			stat_tag,
			stat_tagged,
			stat_team_points,
			stat_total_playtime,
			stat_tranq,
			stat_walkergear_destroy,
			stat_xp,
		};

		return stats;
	}

	std::unordered_set<std::uint32_t>& get_rule_specific_stats()
	{
		static std::unordered_set<std::uint32_t> stats =
		{
			stat_bounty,
			stat_bounty_capture,
			stat_comtix,
			stat_disc_defend,
			stat_disc_pickup,
			stat_disc_steal,
			stat_dm_ticket,
			stat_dom,
			stat_domteam,
			stat_dom_defend,
			stat_dom_neutral,
			stat_missile_damage,
			stat_missile_destroy,
			stat_missile_fulton,
			stat_suicide,
			stat_team_points_available,
			stat_team_points_bounty,
			stat_team_points_cloak,
			stat_team_points_comm,
			stat_team_points_sabotage,
			stat_terminalhack,
		};

		return stats;
	}

	std::unordered_map<std::uint32_t, stat_info_t> load_stats_map()
	{
		std::unordered_map<std::uint32_t, stat_info_t> stats_map;

		for (const auto& stat_id : get_general_stats())
		{
			stat_info_t stat_info{};
			stat_info.is_rule_specific = false;
			stats_map.insert(std::make_pair(stat_id, stat_info));
		}

		for (const auto& stat_id : get_rule_specific_stats())
		{
			stat_info_t stat_info{};
			stat_info.is_rule_specific = true;
			stats_map.insert(std::make_pair(stat_id, stat_info));
		}

		return stats_map;
	}

	std::unordered_map<std::uint32_t, stat_info_t>& get_stats_map()
	{
		static auto stats_map = load_stats_map();
		return stats_map;
	}

	std::uint32_t get_rule_id(const std::uint32_t rule_type)
	{
		switch (rule_type)
		{
		case 0:
			return rule_none;
		case 1:
			return rule_bounty_hunter;
		case 2:
			return rule_comm_control;
		case 3:
			return rule_sabotage;
		case 4:
			return rule_cloak_and_dagger;
		}

		return rule_none;
	}

	GET_FIELD_C(mgo_stat, std::uint64_t, id);
	GET_FIELD_C(mgo_stat, std::uint64_t, player_id);
	GET_FIELD_C(mgo_stat, std::uint32_t, rule_id);
	GET_FIELD_C(mgo_stat, std::uint32_t, stat_id);
	GET_FIELD_C(mgo_stat, std::uint32_t, stat_value);

	namespace impl
	{
		template <database_type_t Type>
		std::uint32_t get_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id)
		{
			return database::access<std::uint32_t>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_stat::table.stat_value)
							.from(mgo_stat::table)
								.where(mgo_stat::table.player_id == player_id && mgo_stat::table.rule_id == rule_id && mgo_stat::table.stat_id == stat_id));

				if (results.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(results.front().stat_value.value());
			});
		}

		template <database_type_t Type>
		bool set_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id, 
			const std::uint32_t stat_value, bool add)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_stat::table.stat_value)
							.from(mgo_stat::table)
								.where(mgo_stat::table.player_id == player_id && mgo_stat::table.rule_id == rule_id && mgo_stat::table.stat_id == stat_id));
				
				if (results.empty())
				{
					auto result = db.get_database<Type>()->operator()(
						sqlpp::insert_into(mgo_stat::table)
							.set(mgo_stat::table.player_id = player_id, mgo_stat::table.rule_id = rule_id, 
								 mgo_stat::table.stat_id = stat_id, mgo_stat::table.stat_value = stat_value));
					return result != 0ull;
				}
				else
				{
					if (add)
					{
						auto result = db.get_database<Type>()->operator()(
							sqlpp::update(mgo_stat::table)
								.set(mgo_stat::table.stat_value = mgo_stat::table.stat_value + stat_value)
									.where(mgo_stat::table.player_id == player_id && mgo_stat::table.rule_id == rule_id && mgo_stat::table.stat_id == stat_id));
						return result != 0ull;
					}
					else
					{
						auto result = db.get_database<Type>()->operator()(
							sqlpp::update(mgo_stat::table)
								.set(mgo_stat::table.stat_value = stat_value)
									.where(mgo_stat::table.player_id == player_id && mgo_stat::table.rule_id == rule_id && mgo_stat::table.stat_id == stat_id));
						return result != 0ull;
					}

				}
			});
		}

		template <database_type_t Type>
		std::vector<mgo_stat> get_stats(const std::uint64_t player_id)
		{
			return database::access<std::vector<mgo_stat>>([&](database_t& db)
				-> std::vector<mgo_stat>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_stat::table))
							.from(mgo_stat::table)
								.where(mgo_stat::table.player_id == player_id));

				std::vector<mgo_stat> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}
	}

	std::uint32_t get_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id)
	{
		RUN_IMPL(impl::get_stat, player_id, rule_id, stat_id);
	}

	std::vector<mgo_stat> get_stats(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_stats, player_id);
	}

	bool set_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id, const std::uint32_t stat_value)
	{
		RUN_IMPL(impl::set_stat, player_id, rule_id, stat_id, stat_value, false);
	}

	bool add_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id, const std::uint32_t stat_value)
	{
		RUN_IMPL(impl::set_stat, player_id, rule_id, stat_id, stat_value, true);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_stats.create");
		}
	};
}

REGISTER_TABLE(database::mgo_stats::table, -1)
