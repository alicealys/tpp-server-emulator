#include <std_include.hpp>

#include "fob_rewards.hpp"
#include "players.hpp"

#include "game/game.hpp"

namespace database::fob_rewards
{
	GET_FIELD_C(fob_reward_group, std::uint64_t, reward_group_id);
	GET_FIELD_C(fob_reward_group, std::uint64_t, attacker_id);
	GET_FIELD_C(fob_reward_group, std::uint64_t, owner_id);
	GET_FIELD_C(fob_reward_group, std::chrono::seconds, create_date);

	GET_FIELD_C(fob_reward, std::uint64_t, reward_id);
	GET_FIELD_C(fob_reward, std::uint64_t, reward_group_id);

	namespace impl
	{
		template <database_type_t Type>
		bool send_rewards(const std::uint64_t owner_id, const std::uint64_t attacker_id, const std::vector<game::reward_t>& rewards)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto insert_group = sqlpp::insert_into(fob_reward_group::table)
					.set(fob_reward_group::table.owner_id = owner_id,
						 fob_reward_group::table.attacker_id = attacker_id,
						 fob_reward_group::table.create_date = std::chrono::system_clock::now());

				const auto group_id = db.exec<Type>(insert_group);
				if (group_id == 0ull)
				{
					return false;
				}

				auto insert_rewards = sqlpp::insert_into(fob_reward::table)
					.columns(
						fob_reward::table.reward_group_id,
						fob_reward::table.reward_param
					);

				for (auto& reward : rewards)
				{
					insert_rewards.values.add(
						fob_reward::table.reward_group_id = std::uint64_t(group_id),
						fob_reward::table.reward_param = utils::encoding::encode_base64(reward)
					);
				}

				const auto result = db.exec<Type>(insert_rewards);
				return result != 0ull;
			});
		}
		
		template <database_type_t Type>
		std::vector<reward_group_t> get_rewards(const std::uint64_t owner_id)
		{
			std::vector<reward_group_t> list;
			reward_group_t current_group{};
			std::uint64_t current_group_id = 0ull;

			database::access([&](database::database_t& db)
			{
				auto join_tables = fob_reward::table
					.join(fob_reward_group::table)
						.on(fob_reward::table.reward_group_id == fob_reward_group::table.reward_group_id)
					.join(players::player::table)
						.on(players::player::table.id == fob_reward_group::table.attacker_id);

				auto select_rewards = sqlpp::select(
						fob_reward_group::table.attacker_id,
						players::player::table.account_id,
						fob_reward::table.reward_group_id,
						fob_reward::table.reward_param)
					.from(join_tables)
						.where(fob_reward_group::table.owner_id == owner_id)
							.order_by(fob_reward::table.reward_group_id.desc());

				auto results = db.exec<Type>(select_rewards);

				for (auto& row : results)
				{
					database_struct<game::reward_t> param{};
					param.deserialize(row.reward_param);

					const std::uint64_t reward_group_id = row.reward_group_id;

					if (reward_group_id != current_group_id)
					{
						if (current_group_id != 0)
						{
							list.emplace_back(current_group);
						}

						current_group.rewards.clear();
						current_group.attacker_id = row.attacker_id;
						current_group.attacker_xuid = row.account_id;
						current_group_id = reward_group_id;
					}

					current_group.rewards.emplace_back(param);
				}
			});

			if (current_group_id != 0)
			{
				list.emplace_back(current_group);
			}

			return list;
		}

		template <database_type_t Type>
		void clear_rewards(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				auto groups = sqlpp::select(fob_reward_group::table.reward_group_id).from(fob_reward_group::table)
					.where(fob_reward_group::table.owner_id == player_id);

				db.exec<Type>(sqlpp::remove_from(fob_reward::table)
					.where(fob_reward::table.reward_group_id.in(groups)));

				db.exec<Type>(sqlpp::remove_from(fob_reward_group::table)
					.where(fob_reward_group::table.owner_id == player_id));
			});
		}
		
		template <database_type_t Type>
		bool has_rewards(const std::uint64_t player_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				const auto results = db.exec<Type>(
					sqlpp::select(fob_reward_group::table.reward_group_id)
						.from(fob_reward_group::table)
							.where(fob_reward_group::table.owner_id == player_id)
								.limit(1u));

				return !results.empty();
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				auto groups = sqlpp::select(fob_reward_group::table.reward_group_id).from(fob_reward_group::table)
					.where(fob_reward_group::table.owner_id == player_id ||
						   fob_reward_group::table.attacker_id == player_id);

				db.exec<Type>(sqlpp::remove_from(fob_reward::table)
					.where(fob_reward::table.reward_group_id.in(groups)));

				db.exec<Type>(sqlpp::remove_from(fob_reward_group::table)
					.where(fob_reward_group::table.attacker_id == player_id || 
						   fob_reward_group::table.owner_id == player_id));
			});
		}
	}

	bool send_rewards(const std::uint64_t owner_id, const std::uint64_t attacker_id, const std::vector<game::reward_t>& rewards)
	{
		RUN_IMPL(impl::send_rewards, owner_id, attacker_id, rewards);
	}

	std::vector<reward_group_t> get_rewards(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::get_rewards, owner_id);
	}

	void clear_rewards(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::clear_rewards, owner_id);
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
	}

	bool has_rewards(const std::uint64_t owner_id)
	{
		RUN_IMPL(impl::has_rewards, owner_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.fob_reward_groups.create");
			database.run_query("mgstpp.fob_rewards.create");
		}
	};
}

REGISTER_TABLE(database::fob_rewards::table, 1)
