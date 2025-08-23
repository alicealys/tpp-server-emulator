#pragma once

#include "../database.hpp"

namespace database::mgo_data
{
	struct character_params
	{
		std::uint32_t last_character_used;
		std::uint32_t bgm_selected;
	};

	class mgo_data
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(last_character_used, sqlpp::integer_unsigned);
		DEFINE_FIELD(bgm_selected, sqlpp::integer_unsigned);
		DEFINE_FIELD(match_auto_leave, sqlpp::integer_unsigned);
		DEFINE_FIELD(match_briefing_time, sqlpp::integer_unsigned);
		DEFINE_FIELD(match_host_comment, sqlpp::integer_unsigned);
		DEFINE_FIELD(match_max_capacity, sqlpp::integer_unsigned);
		DEFINE_FIELD(match_mission_slot_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp_coin, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp_boost_mag, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp_expire_unix_timestamp, sqlpp::integer_unsigned);
		DEFINE_FIELD(rank_xp, sqlpp::integer_unsigned);
		DEFINE_FIELD(xp_boost_mag, sqlpp::integer_unsigned);
		DEFINE_FIELD(xp_expire_unix_timestamp, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_category, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_id_a, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_id_b, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_id_c, sqlpp::integer_unsigned);
		DEFINE_FIELD(survival_ticket_remain, sqlpp::integer_unsigned);
		DEFINE_FIELD(mission_slot_list, sqlpp::integer_unsigned);
		DEFINE_FIELD(mission_player_num, sqlpp::integer_unsigned);
		DEFINE_FIELD(matches_played, sqlpp::integer_unsigned);
		DEFINE_FIELD(matches_abandoned, sqlpp::integer_unsigned);
		DEFINE_FIELD(matches_started, sqlpp::integer_unsigned);
		DEFINE_TABLE(mgo_data, id_field_t, player_id_field_t,
			last_character_used_field_t, bgm_selected_field_t,
			match_auto_leave_field_t, match_briefing_time_field_t, match_host_comment_field_t, match_max_capacity_field_t,
			match_mission_slot_count_field_t,
			gp_coin_field_t, gp_boost_mag_field_t, gp_expire_unix_timestamp_field_t,
			rank_xp_field_t, xp_boost_mag_field_t, xp_expire_unix_timestamp_field_t,
			reward_category_field_t, reward_id_a_field_t, reward_id_b_field_t, reward_id_c_field_t,
			survival_ticket_remain_field_t, mission_slot_list_field_t, mission_player_num_field_t,
			matches_played_field_t, matches_abandoned_field_t, matches_started_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_data(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->last_character_used_ = row.last_character_used;
			this->bgm_selected_ = row.bgm_selected;
			this->match_auto_leave_ = row.match_auto_leave;
			this->match_briefing_time_ = row.match_briefing_time;
			this->match_host_comment_ = row.match_host_comment;
			this->match_max_capacity_ = row.match_max_capacity;
			this->match_mission_slot_count_ = row.match_mission_slot_count;
			this->gp_coin_ = row.gp_coin;
			this->gp_boost_mag_ = row.gp_boost_mag;
			this->gp_expire_unix_timestamp_ = row.gp_expire_unix_timestamp;
			this->rank_xp_ = row.rank_xp;
			this->xp_boost_mag_ = row.xp_boost_mag;
			this->xp_expire_unix_timestamp_ = row.xp_expire_unix_timestamp;
			this->reward_category_ = row.reward_category;
			this->reward_id_a_ = row.reward_id_a;
			this->reward_id_b_ = row.reward_id_b;
			this->reward_id_c_ = row.reward_id_c;
			this->survival_ticket_remain_ = row.survival_ticket_remain;
			this->mission_slot_list_ = row.mission_slot_list;
			this->mission_player_num_ = row.mission_player_num;
			this->matches_played_ = row.matches_played;
			this->matches_abandoned_ = row.matches_abandoned;
			this->matches_started_ = row.matches_started;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, last_character_used);
		GET_FIELD_H(std::uint32_t, bgm_selected);
		GET_FIELD_H(std::uint32_t, match_auto_leave);
		GET_FIELD_H(std::uint32_t, match_briefing_time);
		GET_FIELD_H(std::uint32_t, match_host_comment);
		GET_FIELD_H(std::uint32_t, match_max_capacity);
		GET_FIELD_H(std::uint32_t, match_mission_slot_count);
		GET_FIELD_H(std::uint32_t, gp_coin);
		GET_FIELD_H(std::uint32_t, gp_boost_mag);
		GET_FIELD_H(std::uint32_t, gp_expire_unix_timestamp);
		GET_FIELD_H(std::uint32_t, rank_xp);
		GET_FIELD_H(std::uint32_t, xp_boost_mag);
		GET_FIELD_H(std::uint32_t, xp_expire_unix_timestamp);
		GET_FIELD_H(std::uint32_t, reward_category);
		GET_FIELD_H(std::uint32_t, reward_id_a);
		GET_FIELD_H(std::uint32_t, reward_id_b);
		GET_FIELD_H(std::uint32_t, reward_id_c);
		GET_FIELD_H(std::uint32_t, survival_ticket_remain);
		GET_FIELD_H(std::uint32_t, mission_slot_list);
		GET_FIELD_H(std::uint32_t, mission_player_num);
		GET_FIELD_H(std::uint32_t, matches_played);
		GET_FIELD_H(std::uint32_t, matches_abandoned);
		GET_FIELD_H(std::uint32_t, matches_started);

	};

	bool create(const std::uint64_t player_id);
	std::optional<mgo_data> find(const std::uint64_t player_id);
	std::optional<mgo_data> find_or_create(const std::uint64_t player_id);

	std::uint32_t get_gp_coins(const std::uint64_t player_id);
	bool spend_gp_coins(const std::uint64_t player_id, const std::uint32_t value);
	std::uint32_t add_gp_coins(const std::uint64_t player_id, const std::uint32_t value);

	bool set_values_from_character(const std::uint64_t player_id, const character_params& params);
	bool update_match_stats(const std::uint64_t player_id, const std::uint32_t abandon, const std::uint32_t started, const std::uint32_t played);

	bool set_boost(const std::uint64_t player_id, const std::uint32_t xp_boost_mag, const std::uint32_t gp_boost_mag);
}
