#pragma once

#include "database/models/players.hpp"

#include "../database.hpp"

namespace database::player_records
{
	constexpr const auto lowest_grade = 0u;
	constexpr const auto highest_grade = 11u;
	constexpr const auto min_league_grade = 0u;
	constexpr const auto max_league_grade = 28u;

	class player_record
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(prev_fob_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_point, sqlpp::integer);
		DEFINE_FIELD(fob_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(prev_fob_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(insurance_end, sqlpp::time_point);
		DEFINE_FIELD(league_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(prev_league_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(prev_league_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_point, sqlpp::integer_unsigned);
		DEFINE_FIELD(pf_point, sqlpp::integer_unsigned);
		DEFINE_FIELD(pf_point_add, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_point, sqlpp::integer_unsigned);
		DEFINE_FIELD(playtime, sqlpp::integer_unsigned);
		DEFINE_FIELD(point, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_defense_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_defense_lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_sneak_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_sneak_lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(shield_date, sqlpp::time_point);
		DEFINE_FIELD(has_fob, sqlpp::boolean);
		DEFINE_FIELD(daily_last, sqlpp::time_point);
		DEFINE_FIELD(daily_last_ack, sqlpp::time_point);
		DEFINE_FIELD(daily_total, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_last_ack, sqlpp::time_point);
		DEFINE_FIELD(event_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_offense_force, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_defense_force, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_security_level, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_cumulative_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_last_update, sqlpp::time_point);
		DEFINE_TABLE(player_records, 
			id_field_t,
			player_id_field_t,
			fob_grade_field_t,
			prev_fob_grade_field_t,
			fob_point_field_t,
			fob_rank_field_t,
			prev_fob_rank_field_t,
			insurance_end_field_t,
			league_grade_field_t,
			prev_league_grade_field_t,
			league_rank_field_t,
			prev_league_rank_field_t,
			league_point_field_t,
			pf_point_field_t,
			pf_point_add_field_t,
			event_point_field_t,
			playtime_field_t,
			point_field_t,
			fob_defense_win_field_t,
			fob_defense_lose_field_t,
			fob_sneak_win_field_t,
			fob_sneak_lose_field_t,
			shield_date_field_t,
			has_fob_field_t,
			daily_last_field_t,
			daily_last_ack_field_t,
			daily_total_field_t,
			league_last_ack_field_t,
			event_id_field_t,
			event_offense_force_field_t,
			event_defense_force_field_t,
			event_security_level_field_t,
			event_cumulative_grade_field_t,
			event_last_update_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		player_record(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->fob_grade_ = static_cast<std::uint32_t>(row.fob_grade);
			this->prev_fob_grade_ = static_cast<std::uint32_t>(row.prev_fob_grade);
			this->fob_point_ = static_cast<std::int32_t>(row.fob_point);
			this->fob_rank_ = static_cast<std::uint32_t>(row.fob_rank);
			this->prev_fob_rank_ = static_cast<std::uint32_t>(row.prev_fob_rank);
			this->league_grade_ = static_cast<std::uint32_t>(row.league_grade);
			this->prev_league_grade_ = static_cast<std::uint32_t>(row.prev_league_grade);
			this->league_rank_ = static_cast<std::uint32_t>(row.league_rank);
			this->prev_league_rank_ = static_cast<std::uint32_t>(row.prev_league_rank);
			this->league_point_ = static_cast<std::uint32_t>(row.league_point);
			this->pf_point_ = static_cast<std::uint32_t>(row.pf_point);
			this->pf_point_add_ = static_cast<std::uint32_t>(row.pf_point_add);
			this->event_point_ = static_cast<std::uint32_t>(row.event_point);
			this->playtime_ = static_cast<std::uint32_t>(row.playtime);
			this->point_ = static_cast<std::uint32_t>(row.point);
			this->fob_defense_win_ = static_cast<std::uint32_t>(row.fob_defense_win);
			this->fob_defense_lose_ = static_cast<std::uint32_t>(row.fob_defense_lose);
			this->fob_sneak_win_ = static_cast<std::uint32_t>(row.fob_sneak_win);
			this->fob_sneak_lose_ = static_cast<std::uint32_t>(row.fob_sneak_lose);
			this->has_fob_ = static_cast<bool>(row.has_fob);
			this->daily_last_ = std::chrono::duration_cast<std::chrono::seconds>(row.daily_last.value().time_since_epoch());
			this->daily_last_ack_ = std::chrono::duration_cast<std::chrono::seconds>(row.daily_last_ack.value().time_since_epoch());
			this->daily_total_ = static_cast<std::uint32_t>(row.daily_total);
			this->league_last_ack_ = std::chrono::duration_cast<std::chrono::seconds>(row.league_last_ack.value().time_since_epoch());

			this->shield_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.shield_date.value().time_since_epoch());
			this->insurance_end_ = std::chrono::duration_cast<std::chrono::seconds>(row.insurance_end.value().time_since_epoch());

			this->event_id_ = row.event_id;
			this->event_offense_force_ = static_cast<std::uint32_t>(row.event_offense_force);
			this->event_defense_force_ = static_cast<std::uint32_t>(row.event_defense_force);
			this->event_security_level_ = static_cast<std::uint32_t>(row.event_security_level);
			this->event_cumulative_grade_ = static_cast<std::uint32_t>(row.event_cumulative_grade);
			this->event_last_update_ = std::chrono::duration_cast<std::chrono::seconds>(row.event_last_update.value().time_since_epoch());
		}

		bool has_insurance() const;
		std::int64_t get_shield_date() const;
		bool is_shield_active() const;
		bool has_an_fob() const;

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, fob_grade);
		GET_FIELD_H(std::uint32_t, prev_fob_grade);
		GET_FIELD_H(std::int32_t, fob_point);
		GET_FIELD_H(std::uint32_t, fob_rank);
		GET_FIELD_H(std::uint32_t, prev_fob_rank);
		GET_FIELD_H(std::uint32_t, league_grade);
		GET_FIELD_H(std::uint32_t, prev_league_grade);
		GET_FIELD_H(std::uint32_t, league_rank);
		GET_FIELD_H(std::uint32_t, prev_league_rank);
		GET_FIELD_H(std::uint32_t, league_point);
		GET_FIELD_H(std::uint32_t, pf_point);
		GET_FIELD_H(std::uint32_t, pf_point_add);
		GET_FIELD_H(std::uint32_t, event_point);
		GET_FIELD_H(std::uint32_t, playtime);
		GET_FIELD_H(std::uint32_t, point);
		GET_FIELD_H(std::uint32_t, fob_defense_win);
		GET_FIELD_H(std::uint32_t, fob_defense_lose);
		GET_FIELD_H(std::uint32_t, fob_sneak_win);
		GET_FIELD_H(std::uint32_t, fob_sneak_lose);
		GET_FIELD_H(bool, has_fob);
		GET_FIELD_H(std::uint32_t, daily_total);
		GET_FIELD_H(std::chrono::seconds, daily_last);
		GET_FIELD_H(std::chrono::seconds, daily_last_ack);
		GET_FIELD_H(std::chrono::seconds, league_last_ack);
		GET_FIELD_H(std::chrono::seconds, insurance_end);
		GET_FIELD_H(std::uint64_t, event_id);
		GET_FIELD_H(std::uint32_t, event_offense_force);
		GET_FIELD_H(std::uint32_t, event_defense_force);
		GET_FIELD_H(std::uint32_t, event_security_level);
		GET_FIELD_H(std::uint32_t, event_cumulative_grade);
		GET_FIELD_H(std::chrono::seconds, event_last_update);

	private:
		std::chrono::seconds shield_date_{};

	};

	std::optional<player_record> find(const std::uint64_t player_id);
	player_record find_or_create(const std::uint64_t player_id);
	void add_sneak_result(const std::uint64_t player_id, const std::uint64_t owner_id, 
		const std::int32_t point_add, const bool is_win, const bool is_sneak);
	void sync_prev_values(const std::uint64_t player_id);
	void add_event_points(const std::uint64_t player_id, const std::uint32_t value);
	bool spend_event_points(const std::uint64_t player_id, const std::uint32_t value);
	void reset_event_points();

	std::vector<player_record> find_players_of_grade(const std::uint64_t player_id, const std::uint32_t grade, const std::uint32_t limit);
	std::vector<player_record> find_same_grade_players(const std::uint64_t player_id, const std::uint32_t limit);
	std::vector<player_record> find_higher_grade_players(const std::uint64_t player_id, const std::uint32_t limit);

	void set_shield_date(const std::uint64_t player_id, const bool is_win);
	void clear_shield_date(const std::uint64_t player_id);

	void set_fob_point(const std::uint64_t player_id, const std::uint32_t value);
	void set_has_fob(const std::uint64_t player_id, const bool has_fob);

	void set_daily_login(const std::uint64_t player_id);
	void set_daily_reward(const std::uint64_t player_id);

	void set_insurance(const std::uint64_t player_id, const std::chrono::seconds duration);

	void add_league_points(const std::uint64_t player_id, const std::uint32_t points);
	void inc_league_grade(const std::uint64_t player_id, bool increase);

	void add_pf_points(const std::uint64_t player_id, const std::uint32_t points);
	void set_pf_points(const std::uint64_t player_id, const std::uint32_t points);
	bool spend_pf_points(const std::uint64_t player_id, const std::uint32_t points);

	void update_fob_ranking(database_t& db);
	void update_league_ranking(database_t& db);

	void delete_player_data(const std::uint64_t player_id);
}
