#include <std_include.hpp>

#include "mgo_stats.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_stats
{
	std::unordered_map<std::uint32_t, std::uint32_t>& get_rule_map()
	{
		static std::unordered_map<std::uint32_t, std::uint32_t> map =
		{
			{rule_name_bounty_hunter, rule_bounty_hunter},
			{rule_name_comm_control, rule_comm_control},
			{rule_name_sabotage, rule_sabotage},
			{rule_name_cloak_and_dagger, rule_cloak_and_dagger},
		};

		return map;
	}

	std::unordered_map<std::uint32_t, std::uint32_t> get_rev_map(const std::unordered_map<std::uint32_t, std::uint32_t>& map)
	{
		std::unordered_map<std::uint32_t, std::uint32_t> rev_map;

		for (const auto& [r, l] : map)
		{
			rev_map.insert(std::make_pair(l, r));
		}

		return rev_map;
	}

	std::unordered_map<std::uint32_t, std::uint32_t>& get_rule_map_rev()
	{
		static auto map = get_rev_map(get_rule_map());
		return map;
	}

	std::unordered_map<std::uint32_t, std::uint32_t>& get_stats_map()
	{
		static std::unordered_map<std::uint32_t, std::uint32_t> map =
		{
			{stat_name_abandon_midmatch, stat_abandon_midmatch},
			{stat_name_amazing_throw, stat_amazing_throw},
			{stat_name_ascension_neg_xp, stat_ascension_neg_xp},
			{stat_name_assist, stat_assist},
			{stat_name_calc_xp, stat_calc_xp},
			{stat_name_charm, stat_charm},
			{stat_name_cqc, stat_cqc},
			{stat_name_death, stat_death},
			{stat_name_disconnection, stat_disconnection},
			{stat_name_friend_kill, stat_friend_kill},
			{stat_name_fulton, stat_fulton},
			{stat_name_fultoned, stat_fultoned},
			{stat_name_fultonsave, stat_fultonsave},
			{stat_name_games_won, stat_games_won},
			{stat_name_gear_points, stat_gear_points},
			{stat_name_headshot, stat_headshot},
			{stat_name_host_abandonned, stat_host_abandonned},
			{stat_name_interrogation, stat_interrogation},
			{stat_name_kill, stat_kill},
			{stat_name_knife, stat_knife},
			{stat_name_matches_lost, stat_matches_lost},
			{stat_name_matches_played, stat_matches_played},
			{stat_name_matches_started, stat_matches_started},
			{stat_name_matches_won, stat_matches_won},
			{stat_name_raw_xp, stat_raw_xp},
			{stat_name_stunned, stat_stunned},
			{stat_name_tag, stat_tag},
			{stat_name_tagged, stat_tagged},
			{stat_name_team_points, stat_team_points},
			{stat_name_total_playtime, stat_total_playtime},
			{stat_name_tranq, stat_tranq},
			{stat_name_walkergear_destroy, stat_walkergear_destroy},
			{stat_name_xp, stat_xp},
			{stat_name_bounty, stat_bounty},
			{stat_name_bounty_capture, stat_bounty_capture},
			{stat_name_comtix, stat_comtix},
			{stat_name_disc_defend, stat_disc_defend},
			{stat_name_disc_pickup, stat_disc_pickup},
			{stat_name_disc_steal, stat_disc_steal},
			{stat_name_dm_ticket, stat_dm_ticket},
			{stat_name_dom, stat_dom},
			{stat_name_domteam, stat_domteam},
			{stat_name_dom_defend, stat_dom_defend},
			{stat_name_dom_neutral, stat_dom_neutral},
			{stat_name_missile_damage, stat_missile_damage},
			{stat_name_missile_destroy, stat_missile_destroy},
			{stat_name_missile_fulton, stat_missile_fulton},
			{stat_name_suicide, stat_suicide},
			{stat_name_team_points_available, stat_team_points_available},
			{stat_name_team_points_bounty, stat_team_points_bounty},
			{stat_name_team_points_cloak, stat_team_points_cloak},
			{stat_name_team_points_comm, stat_team_points_comm},
			{stat_name_team_points_sabotage, stat_team_points_sabotage},
			{stat_name_terminalhack, stat_terminalhack},
		};

		return map;
	}

	std::unordered_map<std::uint32_t, std::uint32_t>& get_stats_map_rev()
	{
		static auto map = get_rev_map(get_stats_map());
		return map;
	}

	bool stats_t::parse(nlohmann::json& data)
	{
		auto& rule_stat_list_j = data["rule_stat_list"];
		auto& stat_list_j = data["stat_list"];

		const auto assert_array = [](nlohmann::json& value)
		{
			if (!value.is_array())
			{
				return false;
			}

			for (auto i = 0ull; i < value.size(); i++)
			{
				if (!value[i].is_object())
				{
					return false;
				}
			}

			return true;
		};

		const auto get_value = [&](nlohmann::json& value)
			-> std::uint32_t
		{
			if (value.is_number_unsigned())
			{
				return value.get<std::uint32_t>();
			}

			return 0u;
		};

		if (!assert_array(rule_stat_list_j) || !assert_array(stat_list_j))
		{
			return false;
		}

		const auto& rule_map = get_rule_map();
		const auto& stats_map = get_stats_map();

		const auto insert_stat = [&](const std::uint32_t rule_name, const std::uint32_t stat_name, const std::uint32_t value)
		{
			const auto stat_iter = stats_map.find(stat_name);
			if (stat_iter == stats_map.end())
			{
				return;
			}

			const auto stat_index = stat_iter->second;
			if (rule_name == 0)
			{
				if (stat_index >= stat_base_begin && stat_index <= stat_base_end)
				{
					this->stat_list[stat_index - stat_base_begin] = value;
				}
			}
			else
			{
				const auto rule_iter = rule_map.find(rule_name);
				if (rule_iter == rule_map.end())
				{
					return;
				}

				const auto rule_index = rule_iter->second;
				if (stat_index >= stat_rule_begin && stat_index <= stat_rule_end)
				{
					this->rule_stat_list[rule_index][stat_index - stat_rule_begin] = value;
				}
			}
		};

		for (auto i = 0; i < rule_stat_list_j.size(); i++)
		{
			auto& stat = rule_stat_list_j[i];
			const auto id = get_value(stat["id"]);
			const auto rule_code = get_value(stat["rule_code"]);
			const auto value = get_value(stat["value"]);

			insert_stat(rule_code, id, value);
		}

		for (auto i = 0; i < stat_list_j.size(); i++)
		{
			auto& stat = stat_list_j[i];
			const auto id = get_value(stat["id"]);
			const auto value = get_value(stat["value"]);

			insert_stat(0, id, value);
		}

		return true;
	}

	nlohmann::json stats_t::to_json() const
	{
		nlohmann::json result;

		const auto& rule_map_rev = get_rule_map_rev();
		const auto& stats_map_rev = get_stats_map_rev();

		auto index = 0;
		for (auto o = 0; o < stat_rule_count; o++)
		{
			const auto stat_index = stat_rule_begin + o;
			const auto stat_name = stats_map_rev.at(stat_index);

			for (auto i = 0; i < rule_count; i++)
			{
				const auto rule_name = rule_map_rev.at(i);
				result["rule_stat_list"][index]["id"] = stat_name;
				result["rule_stat_list"][index]["rule_code"] = rule_name;
				result["rule_stat_list"][index]["value"] = this->rule_stat_list[i][o];
				index++;
			}
		}

		for (auto i = 0; i < stat_base_count; i++)
		{
			const auto stat_index = stat_base_begin + i;
			const auto stat_name = stats_map_rev.at(stat_index);

			result["stat_list"][i]["id"] = stat_name;
			result["stat_list"][i]["value"] = this->stat_list[i];
		}

		return result;
	}

	namespace impl
	{
		template <database_type_t Type>
		bool set_stats(const std::uint64_t player_id, const std::shared_ptr<stats_t>& stats)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_stats::table.player_id)
							.from(mgo_stats::table)
								.where(mgo_stats::table.player_id == player_id));
				
				if (results.empty())
				{
					auto result = db.get_database<Type>()->operator()(
						sqlpp::insert_into(mgo_stats::table)
							.set(mgo_stats::table.player_id = player_id, mgo_stats::table.stats = stats->to_binary()));
					return result != 0ull;
				}
				else
				{
					auto result = db.get_database<Type>()->operator()(
						sqlpp::update(mgo_stats::table)
							.set(mgo_stats::table.stats = stats->to_binary())
								.where(mgo_stats::table.player_id == player_id));
					return result != 0ull;
				}
			});
		}
		
		template <database_type_t Type>
		std::shared_ptr<stats_t> get_stats(const std::uint64_t player_id)
		{
			return database::access<std::shared_ptr<stats_t>>([&](database_t& db)
				-> std::shared_ptr<stats_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_stats::table))
							.from(mgo_stats::table)
								.where(mgo_stats::table.player_id == player_id));

				auto stats = std::make_shared<stats_t>();

				if (!results.empty())
				{
					load_binary_field(stats.get(), results.front().stats.value());
				}

				return stats;
			});
		}
	}

	std::shared_ptr<stats_t> get_stats(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_stats, player_id);
	}

	bool set_stats(const std::uint64_t player_id, const std::shared_ptr<stats_t>& stats)
	{
		RUN_IMPL(impl::set_stats, player_id, stats);
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
