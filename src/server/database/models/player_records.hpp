#pragma once

#include "database/models/players.hpp"

#include "../database.hpp"

namespace database::player_records
{
	constexpr const auto lowest_grade = 0;
	constexpr const auto highest_grade = 11;
	constexpr const auto challenge_tasks_count = 32u;

	struct challenge_tasks_t
	{
		std::uint32_t status[challenge_tasks_count];
	};

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
		DEFINE_FIELD(is_insurance, sqlpp::boolean);
		DEFINE_FIELD(league_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(prev_league_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(prev_league_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_point, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_point, sqlpp::integer_unsigned);
		DEFINE_FIELD(playtime, sqlpp::integer_unsigned);
		DEFINE_FIELD(point, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_defense_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_defense_lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_sneak_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_sneak_lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(shield_date, sqlpp::time_point);
		DEFINE_FIELD(has_fob, sqlpp::boolean);
		DEFINE_FIELD(challenge_tasks, sqlpp::binary);
		DEFINE_FIELD(daily_last, sqlpp::time_point);
		DEFINE_FIELD(daily_last_ack, sqlpp::time_point);
		DEFINE_FIELD(daily_total, sqlpp::integer_unsigned);
		DEFINE_TABLE(player_records, id_field_t, player_id_field_t, fob_grade_field_t, prev_fob_grade_field_t,
			fob_point_field_t, fob_rank_field_t, prev_fob_rank_field_t, is_insurance_field_t,
			league_grade_field_t, prev_league_grade_field_t, league_rank_field_t, prev_league_rank_field_t, league_point_field_t, event_point_field_t,
			playtime_field_t, point_field_t, fob_defense_win_field_t, fob_defense_lose_field_t, fob_sneak_win_field_t,
			fob_sneak_lose_field_t, shield_date_field_t, has_fob_field_t, challenge_tasks_field_t, 
			daily_last_field_t, daily_last_ack_field_t, daily_total_field_t);

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
			this->is_insurance_ = static_cast<std::uint32_t>(row.is_insurance);
			this->league_grade_ = static_cast<std::uint32_t>(row.league_grade);
			this->prev_league_grade_ = static_cast<std::uint32_t>(row.prev_league_grade);
			this->league_rank_ = static_cast<std::uint32_t>(row.league_rank);
			this->prev_league_rank_ = static_cast<std::uint32_t>(row.prev_league_rank);
			this->league_point_ = static_cast<std::uint32_t>(row.league_point);
			this->event_point_ = static_cast<std::uint32_t>(row.event_point);
			this->playtime_ = static_cast<std::uint32_t>(row.playtime);
			this->point_ = static_cast<std::uint32_t>(row.point);
			this->fob_defense_win_ = static_cast<std::uint32_t>(row.fob_defense_win);
			this->fob_defense_lose_ = static_cast<std::uint32_t>(row.fob_defense_lose);
			this->fob_sneak_win_ = static_cast<std::uint32_t>(row.fob_sneak_win);
			this->fob_sneak_lose_ = static_cast<std::uint32_t>(row.fob_sneak_lose);
			this->has_fob_ = static_cast<bool>(row.has_fob);

			const auto challenge_tasks_str = row.challenge_tasks.value();
			if (challenge_tasks_str.size() == sizeof(challenge_tasks_t))
			{
				std::memcpy(&this->challenge_tasks_, challenge_tasks_str.data(), sizeof(challenge_tasks_t));
			}

			this->daily_last_ = std::chrono::duration_cast<std::chrono::seconds>(row.daily_last.value().time_since_epoch());
			this->daily_last_ack_ = std::chrono::duration_cast<std::chrono::seconds>(row.daily_last.value().time_since_epoch());
			this->daily_total_ = static_cast<std::uint32_t>(row.daily_total);

			this->shield_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.shield_date.value().time_since_epoch());
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint32_t get_fob_grade() const
		{
			return this->fob_grade_;
		}

		std::uint32_t get_prev_fob_grade() const
		{
			return this->prev_fob_grade_;
		}

		std::int32_t get_fob_point() const
		{
			return this->fob_point_;
		}

		std::uint32_t get_fob_rank() const
		{
			return this->fob_rank_;
		}

		std::uint32_t get_prev_fob_rank() const
		{
			return this->prev_fob_rank_;
		}

		std::uint32_t get_is_insurance() const
		{
			return this->is_insurance_;
		}

		std::uint32_t get_league_grade() const
		{
			return this->league_grade_;
		}

		std::uint32_t get_prev_league_grade() const
		{
			return this->prev_league_grade_;
		}

		std::uint32_t get_league_rank() const
		{
			return this->league_rank_;
		}

		std::uint32_t get_prev_league_rank() const
		{
			return this->prev_league_rank_;
		}

		std::uint32_t get_league_point() const
		{
			return this->league_point_;
		}

		std::uint32_t get_event_point() const
		{
			return this->event_point_;
		}

		std::uint32_t get_playtime() const
		{
			return this->playtime_;
		}

		std::uint32_t get_point() const
		{
			return this->point_;
		}

		std::uint32_t get_defense_win() const
		{
			return this->fob_defense_win_;
		}

		std::uint32_t get_defense_lose() const
		{
			return this->fob_defense_lose_;
		}

		std::uint32_t get_sneak_win() const
		{
			return this->fob_sneak_win_;
		}

		std::uint32_t get_sneak_lose() const
		{
			return this->fob_sneak_lose_;
		}

		std::int64_t get_shield_date() const
		{
			const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
			if (now > this->shield_date_)
			{
				return 0u;
			}

			return this->shield_date_.count();
		}

		int is_shield_active() const
		{
			return this->get_shield_date() != 0;
		}

		bool has_an_fob() const
		{
			return this->has_fob_;
		}

		challenge_tasks_t& get_challenge_tasks()
		{
			return this->challenge_tasks_;
		}

		const challenge_tasks_t& get_challenge_tasks() const
		{
			return this->challenge_tasks_;
		}

		std::chrono::seconds get_daily_last() const
		{
			return this->daily_last_;
		}

		std::chrono::seconds get_daily_last_ack() const
		{
			return this->daily_last_ack_;
		}

		std::uint32_t get_daily_total() const
		{
			return this->daily_total_;
		}

	private:
		std::uint64_t id_{};
		std::uint64_t player_id_{};
		std::uint32_t fob_grade_{};
		std::uint32_t prev_fob_grade_{};
		std::int32_t fob_point_{};
		std::uint32_t fob_rank_{};
		std::uint32_t prev_fob_rank_{};
		std::uint32_t is_insurance_{};
		std::uint32_t league_grade_{};
		std::uint32_t prev_league_grade_{};
		std::uint32_t league_rank_{};
		std::uint32_t prev_league_rank_{};
		std::uint32_t league_point_{};
		std::uint32_t event_point_{};
		std::uint32_t playtime_{};
		std::uint32_t point_{};
		std::uint32_t fob_defense_win_{};
		std::uint32_t fob_defense_lose_{};
		std::uint32_t fob_sneak_win_{};
		std::uint32_t fob_sneak_lose_{};
		challenge_tasks_t challenge_tasks_{};
		bool has_fob_{};
		std::chrono::seconds daily_last_{};
		std::chrono::seconds daily_last_ack_{};
		std::uint32_t daily_total_{};

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

	bool set_challenge_tasks(const std::uint64_t player_id, const challenge_tasks_t& tasks);

	void set_daily_login(const std::uint64_t player_id);
	void set_daily_reward(const std::uint64_t player_id);
}
