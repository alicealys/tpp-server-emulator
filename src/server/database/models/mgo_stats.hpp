#pragma once

#include "../database.hpp"

#include "utils/encoding.hpp"

namespace database::mgo_stats
{
    enum rule_index_t
    {
        rule_bounty_hunter = 0,
        rule_comm_control = 1,
        rule_sabotage = 2,
        rule_cloak_and_dagger = 3,
        rule_count
    };

    enum stat_index_t
    {
        stat_rule_begin = 0,

        stat_abandon_midmatch = 0,
        stat_amazing_throw = 1,
        stat_ascension_neg_xp = 2,
        stat_assist = 3,
        stat_calc_xp = 4,
        stat_charm = 5,
        stat_cqc = 6,
        stat_death = 7,
        stat_disconnection = 8,
        stat_friend_kill = 9,
        stat_fulton = 10,
        stat_fultoned = 11,
        stat_fultonsave = 12,
        stat_games_won = 13,
        stat_gear_points = 14,
        stat_headshot = 15,
        stat_host_abandonned = 16,
        stat_interrogation = 17,
        stat_kill = 18,
        stat_knife = 19,
        stat_matches_lost = 20,
        stat_matches_played = 21,
        stat_matches_started = 22,
        stat_matches_won = 23,
        stat_raw_xp = 24,
        stat_stunned = 25,
        stat_tag = 26,
        stat_tagged = 27,
        stat_team_points = 28,
        stat_total_playtime = 29,
        stat_tranq = 30,
        stat_walkergear_destroy = 31,
        stat_xp = 32,

        stat_rule_end = 32,
        stat_rule_count = 33,

        stat_base_begin = 33,

        stat_bounty = 33,
        stat_bounty_capture = 34,
        stat_comtix = 35,
        stat_disc_defend = 36,
        stat_disc_pickup = 37,
        stat_disc_steal = 38,
        stat_dm_ticket = 39,
        stat_dom = 40,
        stat_domteam = 41,
        stat_dom_defend = 42,
        stat_dom_neutral = 43,
        stat_missile_damage = 44,
        stat_missile_destroy = 45,
        stat_missile_fulton = 46,
        stat_suicide = 47,
        stat_team_points_available = 48,
        stat_team_points_bounty = 49,
        stat_team_points_cloak = 50,
        stat_team_points_comm = 51,
        stat_team_points_sabotage = 52,
        stat_terminalhack = 53,

        stat_base_end = 53,
        stat_base_count = 21
    };

    enum rule_name_t : std::uint32_t
    {
        rule_name_none = 0,
        rule_name_bounty_hunter = 3615292713,
        rule_name_comm_control = 3219621771,
        rule_name_cloak_and_dagger = 3082505545,
        rule_name_sabotage = 4202627005,
        rule_name_count
    };

    enum stat_name_t : std::uint32_t
    {
        stat_name_abandon_midmatch = 1009056792,
        stat_name_amazing_throw = 3074809037,
        stat_name_ascension_neg_xp = 1172841627,
        stat_name_assist = 3011730052,
        stat_name_bounty = 1117213263,
        stat_name_bounty_capture = 2279509158,
        stat_name_calc_xp = 2638259174,
        stat_name_charm = 1264902684,
        stat_name_comtix = 2360290610,
        stat_name_cqc = 759608047,
        stat_name_death = 3509205066,
        stat_name_disconnection = 1898812427,
        stat_name_disc_defend = 914658497,
        stat_name_disc_pickup = 3220552451,
        stat_name_disc_steal = 3391131435,
        stat_name_dm_ticket = 3278021499,
        stat_name_dom = 3685558784,
        stat_name_domteam = 2597492381,
        stat_name_dom_defend = 956134950,
        stat_name_dom_neutral = 3743820509,
        stat_name_friend_kill = 4247973858,
        stat_name_fulton = 4198921810,
        stat_name_fultoned = 3531791035,
        stat_name_fultonsave = 1125549357,
        stat_name_games_won = 4011328285,
        stat_name_gear_points = 433024927,
        stat_name_headshot = 3323394328,
        stat_name_host_abandonned = 3896997883,
        stat_name_interrogation = 1086661461,
        stat_name_kill = 309943336,
        stat_name_knife = 3317856281,
        stat_name_matches_lost = 255959188,
        stat_name_matches_played = 29930463,
        stat_name_matches_started = 1077628973,
        stat_name_matches_won = 1871304466,
        stat_name_missile_damage = 3001637186,
        stat_name_missile_destroy = 2973130442,
        stat_name_missile_fulton = 2228647481,
        stat_name_raw_xp = 1957865976,
        stat_name_stunned = 3622646006,
        stat_name_suicide = 3800451315,
        stat_name_tag = 1369013445,
        stat_name_tagged = 1953425872,
        stat_name_team_points = 3875254222,
        stat_name_team_points_available = 259229151,
        stat_name_team_points_bounty = 1810991501,
        stat_name_team_points_cloak = 1170814218,
        stat_name_team_points_comm = 1274279971,
        stat_name_team_points_sabotage = 1939875265,
        stat_name_terminalhack = 1360877150,
        stat_name_total_playtime = 136521089,
        stat_name_tranq = 3608168627,
        stat_name_walkergear_destroy = 1578514372,
        stat_name_xp = 2208556267,
        stat_name_count
    };

    struct stats_t
    {
        std::uint32_t rule_stat_list[rule_count][stat_rule_count];
        std::uint32_t stat_list[stat_base_count];

        bool parse(nlohmann::json& value);
        nlohmann::json to_json() const;

        auto to_binary() const
        {
            return sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(*this));
        }
    };

	class mgo_stats
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
        DEFINE_FIELD(rule_id, sqlpp::integer_unsigned);
        DEFINE_FIELD(stats, sqlpp::binary);
		DEFINE_TABLE(mgo_stats, id_field_t, player_id_field_t, stats_field_t);

		inline static table_t table;

	};

    std::shared_ptr<stats_t> get_stats(const std::uint64_t player_id);
    bool set_stats(const std::uint64_t player_id, const std::shared_ptr<stats_t>& stats);
    void delete_player_data(const std::uint64_t player_id);
}
