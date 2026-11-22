#pragma once

#include "../database.hpp"
#include "../utils.hpp"

#include "game/game.hpp"

namespace database::fob_rewards
{
	struct reward_group_t
	{
		std::uint64_t attacker_id;
		std::uint64_t attacker_xuid;
		std::vector<game::reward_t> rewards;
	};

	class fob_reward_group
	{
	public:
		DEFINE_FIELD(reward_group_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(owner_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(fob_reward_groups,
			reward_group_id_field_t,
			owner_id_field_t,
			attacker_id_field_t,
			create_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		fob_reward_group(const sqlpp::result_row_t<Args...>& row)
		{
			this->reward_group_id_ = row.reward_group_id;
			this->attacker_id_ = row.attacker_id;
			this->owner_id_ = row.owner_id;
			this->create_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.create_date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, reward_group_id);
		GET_FIELD_H(std::uint64_t, attacker_id);
		GET_FIELD_H(std::uint64_t, owner_id);
		GET_FIELD_H(std::chrono::seconds, create_date);

	};

	class fob_reward
	{
	public:
		DEFINE_FIELD(reward_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_group_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(reward_param, sqlpp::text);
		DEFINE_TABLE(fob_rewards,
			reward_id_field_t,
			reward_group_id_field_t,
			reward_param_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		fob_reward(const sqlpp::result_row_t<Args...>& row)
		{
			this->reward_id_ = row.reward_id;
			this->reward_group_id_ = row.reward_group_id;
			this->param.deserialize(row.reward_param.value());
		}

		GET_FIELD_H(std::uint64_t, reward_id);
		GET_FIELD_H(std::uint64_t, reward_group_id);

		database_struct<game::reward_t> param{};

	};

	bool send_rewards(const std::uint64_t owner_id, const std::uint64_t attacker_id, const std::vector<game::reward_t>& rewards);
	std::vector<reward_group_t> get_rewards(const std::uint64_t owner_id);
	void clear_rewards(const std::uint64_t owner_id);
	bool has_rewards(const std::uint64_t owner_id);

	void delete_player_data(const std::uint64_t player_id);
}
