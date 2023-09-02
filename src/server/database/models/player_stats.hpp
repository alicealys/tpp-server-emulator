#pragma once

#include "../database.hpp"

namespace database::player_stats
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(espionage_lose, sqlpp::integer);
	DEFINE_FIELD(espionage_win, sqlpp::integer);
	DEFINE_FIELD(fob_grade, sqlpp::integer);
	DEFINE_FIELD(fob_point, sqlpp::integer);
	DEFINE_FIELD(fob_rank, sqlpp::integer);
	DEFINE_FIELD(is_insurance, sqlpp::boolean);
	DEFINE_FIELD(league_grade, sqlpp::integral);
	DEFINE_FIELD(league_rank, sqlpp::integer);
	DEFINE_FIELD(playtime, sqlpp::integer);
	DEFINE_FIELD(point, sqlpp::integer);
	DEFINE_TABLE(player_stats, id_t, player_id_t, espionage_lose_t, espionage_win_t, 
		fob_grade_t, fob_point_t, fob_rank_t, is_insurance_t, league_grade_t,
		league_rank_t, playtime_t, point_t);

	class stats
	{
	public:
		template <typename ...Args>
		stats(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->espionage_lose_ = static_cast<std::int32_t>(row.espionage_lose);
			this->espionage_win_ = static_cast<std::int32_t>(row.espionage_win);
			this->fob_grade_ = static_cast<std::int32_t>(row.fob_grade);
			this->fob_point_ = static_cast<std::int32_t>(row.fob_point);
			this->fob_rank_ = static_cast<std::int32_t>(row.fob_rank);
			this->is_insurance_ = static_cast<std::int32_t>(row.is_insurance);
			this->league_grade_ = static_cast<std::int32_t>(row.league_grade);
			this->league_rank_ = static_cast<std::int32_t>(row.league_rank);
			this->playtime_ = static_cast<std::int32_t>(row.playtime);
			this->point_ = static_cast<std::int32_t>(row.point);
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::int32_t get_espionage_lose() const
		{
			return this->espionage_lose_;
		}

		std::int32_t get_espionage_win() const
		{
			return this->espionage_win_;
		}

		std::int32_t get_fob_grade() const
		{
			return this->fob_grade_;
		}

		std::int32_t get_fob_point() const
		{
			return this->fob_point_;
		}

		std::int32_t get_fob_rank() const
		{
			return this->fob_rank_;
		}

		std::int32_t get_is_insurance() const
		{
			return this->is_insurance_;
		}

		std::int32_t get_league_grade() const
		{
			return this->league_grade_;
		}

		std::int32_t get_league_rank() const
		{
			return this->league_rank_;
		}

		std::int32_t get_playtime() const
		{
			return this->playtime_;
		}

		std::int32_t get_point() const
		{
			return this->point_;
		}

	private:
		std::uint64_t id_;
		std::uint64_t player_id_;
		std::int32_t espionage_lose_;
		std::int32_t espionage_win_;
		std::int32_t fob_grade_;
		std::int32_t fob_point_;
		std::int32_t fob_rank_;
		std::int32_t is_insurance_;
		std::int32_t league_grade_;
		std::int32_t league_rank_;
		std::int32_t playtime_;
		std::int32_t point_;

	};

	std::optional<stats> find(const std::uint64_t player_id);
	stats find_or_insert(const std::uint64_t player_id);
}
