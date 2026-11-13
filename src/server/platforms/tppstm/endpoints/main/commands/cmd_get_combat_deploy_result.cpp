#include <std_include.hpp>

#include "cmd_get_combat_deploy_result.hpp"
#include "cmd_get_daily_reward.hpp"

#include "database/models/event_rankings.hpp"

namespace emulator::tpp
{
	bool cmd_get_combat_deploy_result::can_give_reward(const database::player_data::player_data& player_data, const game::reward_t& reward)
	{
		switch (reward.type)
		{
		case 31:
		{
			if (reward.mecha_type == 0)
			{
				return player_data.get_league_attack_item() + reward.value <= database::player_data::league_item_cap;
			}
			else
			{
				return player_data.get_league_defense_item() + reward.value <= database::player_data::league_item_cap;
			}
		}
		default:
			return true;
		}
	}

	nlohmann::json cmd_get_combat_deploy_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		result["result_list"] = nlohmann::json::array();

		const auto& missions = database::combat_deployments::get_mission_list();
		const auto deployments = database::combat_deployments::get_deployments(player->get_id());
		auto i = 0u;

		std::vector<std::uint64_t> remove_list;
		auto win_count = 0;

		for (auto& deployment : deployments)
		{
			if (!deployment.completed())
			{
				continue;
			}

			const auto& deployment_info = deployment.get_deployment_info();
			const auto iter = std::ranges::find_if(missions, [&](const database::combat_deployments::mission_t& mission)
			{
				return mission.id == deployment.get_mission_id();
			});

			if (iter == missions.end())
			{
				continue;
			}

			auto& deployment_j = result["result_list"][i++];
			deployment_j["armored_east"] = deployment_info.armored;
			deployment_j["armored_east_rocket"] = 0;
			deployment_j["armored_west"] = 0;
			deployment_j["armored_west_wheeled"] = 0;
			deployment_j["car_east"] = deployment_info.car;
			deployment_j["car_west"] = 0;
			deployment_j["combat_rank"] = iter->combat_rank;
			deployment_j["damage_seed"] = deployment_info.seed;
			deployment_j["dead_rate"] = deployment_info.dead_rate;
			deployment_j["id"] = deployment.get_mission_id();
			deployment_j["is_win"] = std::uint8_t(deployment.get_is_win());
			deployment_j["name_key"] = iter->name_key;

			deployment_j["staff_count"] = deployment_info.combat_count + deployment_info.sub_count;
			deployment_j["tank_east"] = deployment_info.tank;
			deployment_j["tank_west"] = 0;
			deployment_j["team_id"] = deployment_info.team_id;
			deployment_j["truck_east"] = deployment_info.truck;
			deployment_j["truck_west"] = 0;
			deployment_j["walker_gear_cfa"] = deployment_info.walker_gear;
			deployment_j["walker_gear_cfa_suppot"] = 0;
			deployment_j["walker_gear_proto"] = 0;
			deployment_j["walker_gear_soviet"] = 0;
			deployment_j["walker_gear_soviet_support"] = 0;

			std::vector<game::reward_t> result_rewards;
			if (deployment.get_is_win())
			{
				player_data->give_rewards(iter->rewards, &result_rewards);
				++win_count;
			}

			for (auto o = 0u; o < result_rewards.size(); o++)
			{
				result_rewards[o].to_json(deployment_j["reward"][o]);
			}

			remove_list.emplace_back(deployment.get_id());
		}

		database::event_rankings::increment_event_value(player->get_id(), 
			database::event_rankings::league_dispatch_mission_wins, win_count);
		database::combat_deployments::delete_deployments(remove_list);

		result["result_num"] = i;

		return result;
	}

	bool cmd_get_combat_deploy_result::needs_player()
	{
		return true;
	}
}
