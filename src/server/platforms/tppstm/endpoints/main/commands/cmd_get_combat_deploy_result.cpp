#include <std_include.hpp>

#include "cmd_get_combat_deploy_result.hpp"

#include "database/models/combat_deployments.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_combat_deploy_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		result["result_list"] = nlohmann::json::array();

		const auto& missions = database::combat_deployments::get_mission_list();
		const auto deployments = database::combat_deployments::get_deployments(player->get_id());
		auto i = 0u;

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
			deployment_j["is_win"] = deployment.get_is_win() ? 1 : 0;
			deployment_j["name_key"] = iter->name_key;

			for (auto o = 0u; o < iter->rewards.size(); o++)
			{
				deployment_j["reward"][o]["bottom_type"] = iter->rewards[o].bottom_type;
				deployment_j["reward"][o]["mecha_type"] = iter->rewards[o].mecha_type;
				deployment_j["reward"][o]["rate"] = iter->rewards[o].rate;
				deployment_j["reward"][o]["section"] = iter->section;
				deployment_j["reward"][o]["type"] = iter->reward;
				deployment_j["reward"][o]["value"] = iter->rewards[o].value;
			}

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

			database::combat_deployments::delete_deployment(player->get_id(), deployment.get_mission_id());
		}

		result["result_num"] = i;

		return result;
	}

	bool cmd_get_combat_deploy_result::needs_player()
	{
		return true;
	}
}
