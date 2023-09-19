#pragma once

#include "../database.hpp"

namespace database::player_stats
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_grade, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_point, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_rank, sqlpp::integer_unsigned);
	DEFINE_FIELD(is_insurance, sqlpp::boolean);
	DEFINE_FIELD(league_grade, sqlpp::integer_unsigned);
	DEFINE_FIELD(league_rank, sqlpp::integer_unsigned);
	DEFINE_FIELD(playtime, sqlpp::integer_unsigned);
	DEFINE_FIELD(point, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_defense_win, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_defense_lose, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_sneak_win, sqlpp::integer_unsigned);
	DEFINE_FIELD(fob_sneak_lose, sqlpp::integer_unsigned);
	DEFINE_TABLE(player_stats, id_field_t, player_id_field_t, fob_grade_field_t, 
		fob_point_field_t, fob_rank_field_t, is_insurance_field_t, league_grade_field_t, 
		league_rank_field_t, playtime_field_t, point_field_t, fob_defense_win_field_t, 
		fob_defense_lose_field_t, fob_sneak_win_field_t, fob_sneak_lose_field_t);

	class stats
	{
	public:
		template <typename ...Args>
		stats(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->fob_grade_ = static_cast<std::uint32_t>(row.fob_grade);
			this->fob_point_ = static_cast<std::uint32_t>(row.fob_point);
			this->fob_rank_ = static_cast<std::uint32_t>(row.fob_rank);
			this->is_insurance_ = static_cast<std::uint32_t>(row.is_insurance);
			this->league_grade_ = static_cast<std::uint32_t>(row.league_grade);
			this->league_rank_ = static_cast<std::uint32_t>(row.league_rank);
			this->playtime_ = static_cast<std::uint32_t>(row.playtime);
			this->point_ = static_cast<std::uint32_t>(row.point);
			this->fob_defense_win_ = static_cast<std::uint32_t>(row.fob_defense_win);
			this->fob_defense_lose_ = static_cast<std::uint32_t>(row.fob_defense_lose);
			this->fob_sneak_win_ = static_cast<std::uint32_t>(row.fob_sneak_win);
			this->fob_sneak_lose_ = static_cast<std::uint32_t>(row.fob_sneak_lose);
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint32_t get_fob_grade() const
		{
			return this->fob_grade_;
		}

		std::uint32_t get_fob_point() const
		{
			return this->fob_point_;
		}

		std::uint32_t get_fob_rank() const
		{
			return this->fob_rank_;
		}

		std::uint32_t get_is_insurance() const
		{
			return this->is_insurance_;
		}

		std::uint32_t get_league_grade() const
		{
			return this->league_grade_;
		}

		std::uint32_t get_league_rank() const
		{
			return this->league_rank_;
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
	private:
		std::uint64_t id_;
		std::uint64_t player_id_;
		std::uint32_t fob_grade_;
		std::uint32_t fob_point_;
		std::uint32_t fob_rank_;
		std::uint32_t is_insurance_;
		std::uint32_t league_grade_;
		std::uint32_t league_rank_;
		std::uint32_t playtime_;
		std::uint32_t point_;
		std::uint32_t fob_defense_win_;
		std::uint32_t fob_defense_lose_;
		std::uint32_t fob_sneak_win_;
		std::uint32_t fob_sneak_lose_;

	};

	std::optional<stats> find(const std::uint64_t player_id);
	stats find_or_insert(const std::uint64_t player_id);
}
