#pragma once

#include "../database.hpp"

#include "players.hpp"
#include "player_records.hpp"

namespace database::player_follows
{
	constexpr const auto max_follows = 30u;

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
			: player(row), record(row)
		{
			this->player_id_ = row.player_id;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		players::player player;
		player_records::player_record record;

	private:
		std::uint64_t player_id_{};

	};

	class follow_list : public std::vector<player_follow>
	{
	public:
		bool contains(const std::uint64_t player_id) const;
	};

	bool add_follow(const std::uint64_t player_id, const std::uint64_t to_player_id);
	bool remove_follow(const std::uint64_t player_id, const std::uint64_t to_player_id);

	follow_list get_follows(const std::uint64_t player_id);
	follow_list get_followers(const std::uint64_t to_player_id);
	void delete_player_data(const std::uint64_t player_id);
}
