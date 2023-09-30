#pragma once

#include "../database.hpp"

#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::wormholes
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(to_player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(retaliate_score, sqlpp::integer_unsigned);
	DEFINE_FIELD(flag, sqlpp::integer_unsigned);
	DEFINE_FIELD(is_open, sqlpp::boolean);
	DEFINE_FIELD(create_date, sqlpp::time_point);
	DEFINE_TABLE(wormholes, id_field_t, player_id_field_t, to_player_id_field_t, 
		retaliate_score_field_t, flag_field_t, is_open_field_t, create_date_field_t);

	enum wormhole_flag
	{
		wormhole_flag_invalid,
		wormhole_flag_black,
		wormhole_flag_friendly,
		wormhole_flag_count
	};

	constexpr auto wormhole_duration = 24h * 31;

	class wormhole
	{
	public:
		template <typename ...Args>
		wormhole(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->to_player_id_ = row.to_player_id;
			this->retaliate_score_ = static_cast<std::uint32_t>(row.retaliate_score);
			this->flag_ = static_cast<wormhole_flag>(static_cast<std::uint32_t>(row.flag));
			this->is_open_ = row.is_open;
			this->create_date_ = row.create_date.value().time_since_epoch();
		}

		std::uint64_t get_id() const
		{
			return this->id_;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint64_t get_to_player_id() const
		{
			return this->to_player_id_;
		}

		std::uint32_t get_retaliate_score() const
		{
			return this->retaliate_score_;
		}

		bool is_open() const
		{
			return this->is_open_;
		}

		wormhole_flag get_flag() const
		{
			return this->flag_;
		}

		std::chrono::microseconds get_create_date() const
		{
			return this->create_date_;
		}

	private:
		std::uint64_t id_;
		std::uint64_t player_id_;
		std::uint64_t to_player_id_;
		std::uint32_t retaliate_score_;
		wormhole_flag flag_;
		bool is_open_;
		std::chrono::microseconds create_date_;

	};

	struct wormhole_status
	{
		std::uint64_t player_id;
		std::uint64_t to_player_id;
		std::uint32_t score;
		bool open;
		bool first;
		std::chrono::microseconds expire;
	};

	wormhole_flag get_flag_id(const std::string& flag);

	void add_wormhole(const std::uint64_t player_id, const std::uint64_t to_player_id,
		const wormhole_flag flag, const bool is_open, const std::uint32_t retaliate_point);

	std::unordered_map<std::uint64_t, wormhole_status> find_active_wormholes(const std::uint64_t player_id);

	wormhole_status get_wormhole_status(const std::uint64_t from_player_id, const std::uint64_t to_player_id);
}
