#pragma once

#include "../database.hpp"

#include "players.hpp"

#include "utils/tpp.hpp"

namespace database::player_follows
{
	constexpr auto max_follows = 30;

	class player_follow
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(to_player_id, sqlpp::integer_unsigned);
		DEFINE_TABLE(player_follows, id_field_t, player_id_field_t, to_player_id_field_t);

		inline static table_t table;

		template <typename ...Args>
		player_follow(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->to_player_id_ = row.to_player_id;
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


	private:
		std::uint64_t id_;
		std::uint64_t player_id_;
		std::uint64_t to_player_id_;

	};

	bool add_follow(const std::uint64_t player_id, const std::uint64_t to_player_id);
	bool remove_follow(const std::uint64_t player_id, const std::uint64_t to_player_id);

	std::unordered_set<std::uint64_t> get_follows(const std::uint64_t player_id);
	std::unordered_set<std::uint64_t> get_followers(const std::uint64_t to_player_id);
}
