#pragma once

#include "../database.hpp"

#include "player_data.hpp"

namespace database::wormholes
{
	constexpr const auto mid_retaliate_score = 300u;
	constexpr const auto max_retaliate_score = 720u;

	enum wormhole_flag
	{
		wormhole_flag_invalid,
		wormhole_flag_black,
		wormhole_flag_friendly,
		wormhole_flag_count
	};

	struct wormhole_status_t
	{
		std::uint64_t opponent_id;
		std::int32_t score;
		std::int32_t opponent_score;
		std::int32_t follower_score;
		std::uint32_t attack_count;
		std::uint32_t retaliate_count;
		bool open;
		bool first;
		std::chrono::seconds expire_date;
		std::chrono::seconds last_date;
		std::chrono::seconds first_date;

		bool is_expired() const;
		bool is_open(const bool from_follower = false) const;
		std::uint64_t get_left_hour() const;
	};

	class wormhole
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(to_player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(follower_player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(retaliate_score, sqlpp::integer);
		DEFINE_FIELD(flag, sqlpp::integer_unsigned);
		DEFINE_FIELD(is_open, sqlpp::boolean);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(wormholes, 
			id_field_t, 
			player_id_field_t, 
			to_player_id_field_t,
			follower_player_id_field_t,
			retaliate_score_field_t, 
			flag_field_t, 
			is_open_field_t, 
			create_date_field_t
		);

		inline static table_t table;

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

	wormhole_flag get_flag_id(const std::string& flag);

	void add_wormhole(const std::uint64_t player_id, const std::uint64_t to_player_id, const std::uint64_t follower_id,
		const wormhole_flag flag, const bool is_open, const std::uint32_t retaliate_point);
	void remove_wormholes(const std::uint64_t player_id, const std::uint64_t to_player_id);

	std::vector<wormhole_status_t> get_wormhole_list(const std::uint64_t to_player_id, const std::uint32_t flag = wormhole_flag_invalid, const std::uint64_t follower_id = 0u);
	wormhole_status_t get_wormhole_status(const std::uint64_t from_player_id, const std::uint64_t to_player_id, 
		const std::uint32_t flag = wormhole_flag_invalid);
	
	void delete_player_data(const std::uint64_t player_id);
}
