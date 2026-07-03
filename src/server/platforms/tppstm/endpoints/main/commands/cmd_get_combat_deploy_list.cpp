#include <std_include.hpp>

#include "cmd_get_combat_deploy_list.hpp"

#include "database/models/combat_deployments.hpp"

#include <utils/cryptography.hpp>

namespace emulator::tpp
{
	nlohmann::json cmd_get_combat_deploy_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& mission_list = database::combat_deployments::get_mission_list();
		const auto deployments = database::combat_deployments::get_deployments(player->get_id());

		result["mission_list"] = nlohmann::json::array();
		result["mission_num"] = mission_list.size();

		for (auto i = 0u; i < mission_list.size(); i++)
		{
			auto& mission_j = result["mission_list"][i];
            mission_j["armored_max"] = 0;
            mission_j["armored_min"] = 0;
            mission_j["battle_gear"] = 0;
            mission_j["car_max"] = 0;
            mission_j["car_min"] = 0;
            mission_j["category"] = mission_list[i].category;
            mission_j["combat_count"] = mission_list[i].combat_count;
            mission_j["combat_rank"] = mission_list[i].combat_rank;
            mission_j["dead_rate"] = mission_list[i].dead_rate;
            mission_j["is_campaign"] = mission_list[i].is_campaign;
            mission_j["latitude"] = mission_list[i].latitude;
            mission_j["longitude"] = mission_list[i].longitude;
            mission_j["max_dead_rate"] = mission_list[i].max_dead_rate;
            mission_j["max_win_rate"] = mission_list[i].max_win_rate;
            mission_j["min_dead_rate"] = mission_list[i].min_dead_rate;
            mission_j["min_win_rate"] = mission_list[i].min_win_rate;
            mission_j["mission_id"] = mission_list[i].id;
            mission_j["name_key"] = mission_list[i].name_key;
            mission_j["reward"] = mission_list[i].reward;
            mission_j["section"] = mission_list[i].section;
            mission_j["section_count"] = mission_list[i].section_count;
            mission_j["section_rank"] = mission_list[i].section_rank;
            mission_j["seed"] = utils::cryptography::random::get_integer(1000u, 100000u);
            mission_j["server_text_id"] = 0;
            mission_j["tank_max"] = 0;
            mission_j["tank_min"] = 0;
            mission_j["time"] = mission_list[i].time;
            mission_j["time_random"] = 0;
            mission_j["truck_max"] = 0;
            mission_j["truck_min"] = 0;
            mission_j["walker_gear_max"] = 0;
            mission_j["walker_gear_min"] = 0;
            mission_j["win_rate"] = 100;
            mission_j["primary_reward"] = nlohmann::json::array();

            for (auto o = 0u; o < mission_list[i].rewards.size(); o++)
            {
                mission_j["primary_reward"][o]["bottom_type"] = mission_list[i].rewards[o].bottom_type;
                mission_j["primary_reward"][o]["mecha_type"] = mission_list[i].rewards[o].mecha_type;
                mission_j["primary_reward"][o]["rate"] = mission_list[i].rewards[o].rate;
                mission_j["primary_reward"][o]["section"] = mission_list[i].rewards[o].section;
                mission_j["primary_reward"][o]["type"] = mission_list[i].rewards[o].type;
                mission_j["primary_reward"][o]["value"] = mission_list[i].rewards[o].value;
            }

            const auto iter = std::ranges::find_if(deployments, [&](const database::combat_deployments::combat_deployment& deployment)
            {
                return deployment.get_mission_id() == mission_list[i].id;
            });

            auto& team_j = mission_j["team"];

            if (iter != deployments.end())
            {
                const auto time_left = iter->get_end_date() - iter->get_start_date();
                const auto time_left_s = std::chrono::duration_cast<std::chrono::seconds>(time_left);

                const auto& deployment_info = iter->get_deployment_info();
                team_j["armored"] = deployment_info.armored;
                team_j["battle_gear"] = deployment_info.battle_gear;
                team_j["car"] = deployment_info.car;
                team_j["combat_count"] = deployment_info.combat_count;
                team_j["combat_rank_bottom"] = deployment_info.combat_rank_bottom;
                team_j["combat_rank_top"] = deployment_info.combat_rank_top;
                team_j["dead_rate"] = deployment_info.dead_rate;
                team_j["is_valid"] = 1;
                team_j["mission_id"] = mission_list[i].id;
                team_j["seed"] = deployment_info.seed;
                team_j["staff_power"] = deployment_info.staff_power;
                team_j["sub_count"] = deployment_info.sub_count;
                team_j["sub_rank_bottom"] = deployment_info.sub_rank_bottom;
                team_j["sub_rank_top"] = deployment_info.sub_rank_top;
                team_j["tank"] = deployment_info.tank;
                team_j["team_id"] = deployment_info.team_id;
                team_j["team_power"] = deployment_info.team_power;
                team_j["time"] = time_left_s.count();
                team_j["truck"] = deployment_info.truck;
                team_j["walker_gear"] = deployment_info.walker_gear;
                team_j["win_rate"] = deployment_info.win_rate;
            }
            else
            {
                team_j["armored"] = 0;
                team_j["battle_gear"] = 0;
                team_j["car"] = 0;
                team_j["combat_count"] = 0;
                team_j["combat_rank_bottom"] = 0;
                team_j["combat_rank_top"] = 0;
                team_j["dead_rate"] = 0;
                team_j["is_valid"] = 0;
                team_j["mission_id"] = 0;
                team_j["seed"] = 0;
                team_j["staff_power"] = 0;
                team_j["sub_count"] = 0;
                team_j["sub_rank_bottom"] = 0;
                team_j["sub_rank_top"] = 0;
                team_j["tank"] = 0;
                team_j["team_id"] = 0;
                team_j["team_power"] = 0;
                team_j["time"] = 0;
                team_j["truck"] = 0;
                team_j["walker_gear"] = 0;
                team_j["win_rate"] = 0;
            }
		}

		return result;
	}

	bool cmd_get_combat_deploy_list::needs_player()
	{
		return true;
	}
}
