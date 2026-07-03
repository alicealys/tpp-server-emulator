#include <std_include.hpp>

#include "combat_deployments.hpp"

#include "utils/encoding.hpp"

#include <utils/string.hpp>

namespace database::combat_deployments
{
	namespace
	{
		std::vector<mission_t> parse_mission_list()
		{
			std::vector<mission_t> result;

			const auto list = utils::resources::load_json(RESOURCE_COMBAT_DEPLOY_LIST);

			for (auto& entry : list)
			{
				mission_t mission{};
				mission.id = entry["mission_id"].get<std::uint32_t>();
				mission.category = entry["category"].get<std::uint8_t>();
				mission.combat_count = entry["combat_count"].get<std::uint8_t>();
				mission.combat_rank = entry["combat_rank"].get<std::uint8_t>();
				mission.dead_rate = entry["dead_rate"].get<std::uint8_t>();
				mission.is_campaign = entry["is_campaign"].get<std::uint8_t>();
				mission.latitude = entry["latitude"].get<std::int32_t>();
				mission.longitude = entry["longitude"].get<std::int32_t>();
				mission.max_dead_rate = entry["max_dead_rate"].get<std::uint8_t>();
				mission.max_win_rate = entry["max_win_rate"].get<std::uint8_t>();
				mission.min_dead_rate = entry["min_dead_rate"].get<std::uint8_t>();
				mission.min_win_rate = entry["min_win_rate"].get<std::uint8_t>();
				mission.name_key = entry["name_key"].get<std::uint8_t>();
				mission.reward = entry["reward"].get<std::uint8_t>();
				mission.section = entry["section"].get<std::uint8_t>();
				mission.section_count = entry["section_count"].get<std::uint8_t>();
				mission.section_rank = entry["section_rank"].get<std::uint8_t>();
				mission.time = entry["time"].get<std::uint32_t>();

				auto& rewards_list_j = entry["primary_reward"];
				if (rewards_list_j.is_array())
				{
					for (auto i = 0u; i < rewards_list_j.size(); i++)
					{
						mission_reward_t reward{};
						reward.bottom_type = rewards_list_j[i]["bottom_type"].get<std::uint8_t>();
						reward.mecha_type = rewards_list_j[i]["mecha_type"].get<std::uint8_t>();
						reward.type = rewards_list_j[i]["type"].get<std::uint8_t>();
						reward.section = rewards_list_j[i]["section"].get<std::uint8_t>();
						reward.rate = rewards_list_j[i]["rate"].get<std::uint32_t>();
						reward.value = rewards_list_j[i]["value"].get<std::uint32_t>();

						mission.rewards.emplace_back(reward);
					}
				}

				result.emplace_back(mission);
			}

			return result;
		}
	}

	const std::vector<mission_t>& get_mission_list()
	{
		static auto mission_list = parse_mission_list();
		return mission_list;
	}

	std::optional<mission_t> get_mission(const std::uint32_t mission_id)
	{
		const auto& list = get_mission_list();
		const auto iter = std::ranges::find_if(list.begin(), list.end(), [&](const mission_t& mission)
		{
			return mission.id == mission_id;
		});

		if (iter == list.end())
		{
			return {};
		}

		return {*iter};
	}

	GET_FIELD_C(combat_deployment, std::uint64_t, id);
	GET_FIELD_C(combat_deployment, std::uint64_t, player_id);
	GET_FIELD_C(combat_deployment, std::uint32_t, mission_id);
	GET_FIELD_C(combat_deployment, bool, is_win);
	GET_FIELD_C(combat_deployment, std::chrono::seconds, start_date);
	GET_FIELD_C(combat_deployment, std::chrono::seconds, end_date);

	bool combat_deployment::completed() const
	{
		return this->get_time_left() <= 0s;
	}

	std::chrono::seconds combat_deployment::get_time_left() const
	{
		const auto now = std::chrono::system_clock::now().time_since_epoch();
		const auto now_s = std::chrono::duration_cast<std::chrono::seconds>(now);
		if (now_s > this->end_date_)
		{
			return 0s;
		}

		const auto time_left = this->end_date_ - now_s;
		return time_left;
	}

	namespace impl
	{
		template <database_type_t Type>
		std::vector<combat_deployment> get_deployments(const std::uint64_t player_id)
		{
			return database::access<std::vector<combat_deployment>>([&](database::database_t& db)
				-> std::vector<combat_deployment>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(combat_deployment::table))
							.from(combat_deployment::table)
								.where(combat_deployment::table.player_id == player_id)
									.order_by(combat_deployment::table.start_date.asc()));

				std::vector<combat_deployment> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::optional<combat_deployment> get_deployment(const std::uint64_t player_id, const std::uint32_t mission_id)
		{
			return database::access<std::optional<combat_deployment>>([&](database::database_t& db)
				-> std::optional<combat_deployment>
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(combat_deployment::table))
							.from(combat_deployment::table)
								.where(combat_deployment::table.player_id == player_id && combat_deployment::table.mission_id == mission_id));

				if (result.empty())
				{
					return {};
				}

				return combat_deployment(result.front());
			});
		}

		template <database_type_t Type>
		bool deploy_mission(const std::uint64_t player_id, const std::uint32_t mission_id, const std::uint32_t time, 
			const deployment_info_t& deployment_info, const bool is_win)
		{
			const auto start = std::chrono::system_clock::now();
			const auto end = start + 1s * time;

			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(combat_deployment::table)
						.set(combat_deployment::table.player_id = player_id,
							 combat_deployment::table.mission_id = mission_id,
							 combat_deployment::table.deployment_info = sqlpp::verbatim<sqlpp::binary>(utils::encoding::encode_binary(deployment_info)),
							 combat_deployment::table.is_win = is_win,
							 combat_deployment::table.start_date = start,
							 combat_deployment::table.end_date = end
					));

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool delete_deployment(const std::uint64_t player_id, const std::uint32_t mission_id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::remove_from(combat_deployment::table)
						.where(combat_deployment::table.player_id == player_id &&
							 combat_deployment::table.mission_id == mission_id));

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool delete_all_deployments(const std::uint64_t player_id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::remove_from(combat_deployment::table)
						.where(combat_deployment::table.player_id == player_id));

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool complete_deployment(const std::uint64_t player_id, const std::uint32_t mission_id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(combat_deployment::table)
						.set(combat_deployment::table.end_date = combat_deployment::table.start_date)
							.where(combat_deployment::table.player_id == player_id &&
								combat_deployment::table.mission_id == mission_id));

				return result != 0;
			});
		}
	}

	std::vector<combat_deployment> get_deployments(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_deployments, player_id);
	}

	std::optional<combat_deployment> get_deployment(const std::uint64_t player_id, const std::uint32_t mission_id)
	{
		RUN_IMPL(impl::get_deployment, player_id, mission_id);
	}

	bool deploy_mission(const std::uint64_t player_id, const std::uint32_t mission_id, const std::uint32_t time, 
		const deployment_info_t& deployment_info, const bool is_win)
	{
		RUN_IMPL(impl::deploy_mission, player_id, mission_id, time, deployment_info, is_win);
	}

	bool delete_deployment(const std::uint64_t player_id, const std::uint32_t mission_id)
	{
		RUN_IMPL(impl::delete_deployment, player_id, mission_id);
	}

	bool delete_all_deployments(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_all_deployments, player_id);
	}

	bool complete_deployment(const std::uint64_t player_id, const std::uint32_t mission_id)
	{
		RUN_IMPL(impl::complete_deployment, player_id, mission_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.combat_deployments.create");
		}
	};
}

REGISTER_TABLE(database::combat_deployments::table, -1)
