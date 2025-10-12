#pragma once

#include "../database.hpp"

namespace database::mgo_stats
{
    enum rule_string_id : std::uint32_t
    {
        rule_none = 0,
        rule_bounty_hunter = 3615292713,
        rule_comm_control = 3219621771,
        rule_cloak_and_dagger = 3082505545,
        rule_sabotage = 4202627005,
        rule_count
    };

    enum stat_string_id : std::uint32_t
    {
        stat_abandon_midmatch = 1009056792,
        stat_amazing_throw = 3074809037,
        stat_ascension_neg_xp = 1172841627,
        stat_assist = 3011730052,
        stat_bounty = 1117213263,
        stat_bounty_capture = 2279509158,
        stat_calc_xp = 2638259174,
        stat_charm = 1264902684,
        stat_comtix = 2360290610,
        stat_cqc = 759608047,
        stat_death = 3509205066,
        stat_disconnection = 1898812427,
        stat_disc_defend = 914658497,
        stat_disc_pickup = 3220552451,
        stat_disc_steal = 3391131435,
        stat_dm_ticket = 3278021499,
        stat_dom = 3685558784,
        stat_domteam = 2597492381,
        stat_dom_defend = 956134950,
        stat_dom_neutral = 3743820509,
        stat_friend_kill = 4247973858,
        stat_fulton = 4198921810,
        stat_fultoned = 3531791035,
        stat_fultonsave = 1125549357,
        stat_games_won = 4011328285,
        stat_gear_points = 433024927,
        stat_headshot = 3323394328,
        stat_host_abandonned = 3896997883,
        stat_interrogation = 1086661461,
        stat_kill = 309943336,
        stat_knife = 3317856281,
        stat_matches_lost = 255959188,
        stat_matches_played = 29930463,
        stat_matches_started = 1077628973,
        stat_matches_won = 1871304466,
        stat_missile_damage = 3001637186,
        stat_missile_destroy = 2973130442,
        stat_missile_fulton = 2228647481,
        stat_raw_xp = 1957865976,
        stat_stunned = 3622646006,
        stat_suicide = 3800451315,
        stat_tag = 1369013445,
        stat_tagged = 1953425872,
        stat_team_points = 3875254222,
        stat_team_points_available = 259229151,
        stat_team_points_bounty = 1810991501,
        stat_team_points_cloak = 1170814218,
        stat_team_points_comm = 1274279971,
        stat_team_points_sabotage = 1939875265,
        stat_terminalhack = 1360877150,
        stat_total_playtime = 136521089,
        stat_tranq = 3608168627,
        stat_walkergear_destroy = 1578514372,
        stat_xp = 2208556267,
        stat_count
    };

    struct stat_info_t
    {
        bool is_rule_specific;
    };

    std::unordered_set<std::uint32_t>& get_rules();
    std::unordered_set<std::uint32_t>& get_rule_specific_stats();
    std::unordered_set<std::uint32_t>& get_general_stats();
    std::unordered_map<std::uint32_t, stat_info_t>& get_stats_map();

    std::uint32_t get_rule_id(const std::uint32_t rule_type);

	class mgo_stat
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
        DEFINE_FIELD(rule_id, sqlpp::integer_unsigned);
        DEFINE_FIELD(stat_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(stat_value, sqlpp::integer_unsigned);
		DEFINE_TABLE(mgo_stats, id_field_t, player_id_field_t, rule_id_field_t, stat_id_field_t, stat_value_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_stat(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
            this->rule_id_ = row.rule_id;
            this->stat_id_ = row.stat_id;
			this->stat_value_ = row.stat_value;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
        GET_FIELD_H(std::uint32_t, rule_id);
        GET_FIELD_H(std::uint32_t, stat_id);
		GET_FIELD_H(std::uint32_t, stat_value);

	};

	std::uint32_t get_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id);
    bool set_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id, const std::uint32_t stat_value);
    bool add_stat(const std::uint64_t player_id, const std::uint32_t rule_id, const std::uint32_t stat_id, const std::uint32_t stat_value);

	std::vector<mgo_stat> get_stats(const std::uint64_t player_id);
}
