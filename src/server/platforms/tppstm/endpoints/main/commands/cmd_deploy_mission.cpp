#include <std_include.hpp>

#include "cmd_deploy_mission.hpp"

#include "database/models/combat_deployments.hpp"
#include "cmd_get_combat_deploy_result.hpp"

#include <utils/cryptography.hpp>

namespace emulator::tpp
{
	nlohmann::json cmd_deploy_mission::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& team_j = data["team"];
		if (!team_j.is_object())
		{
			return error(ERR_INVALIDARG);
		}

		const auto get_arg = [&](const std::string& name)
			-> std::uint8_t
		{
			auto& value_j = team_j[name];
			if (!value_j.is_number_unsigned())
			{
				return 0u;
			}

			return value_j.get<std::uint8_t>();
		};

		const auto get_arg32 = [&](const std::string& name)
		{
			auto& value_j = team_j[name];
			if (!value_j.is_number_unsigned())
			{
				return 0u;
			}

			return value_j.get<std::uint32_t>();
		};

		database::combat_deployments::deployment_info_t deployment_info{};
		auto& mission_id_j = team_j["mission_id"];
		if (!mission_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto mission_id = mission_id_j.get<std::uint32_t>();
		const auto mission = database::combat_deployments::get_mission(mission_id);
		if (!mission.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		auto can_give_reward = true;
		for (auto o = 0u; o < mission->rewards.size(); o++)
		{
			if (!cmd_get_combat_deploy_result::can_give_reward(player_data.value(), mission->rewards[o]))
			{
				can_give_reward = false;
				break;
			}
		}

		if (!can_give_reward)
		{
			return error(ERR_DATABASE);
		}

		const auto current_deployment = database::combat_deployments::get_deployment(player->get_id(), mission_id);
		if (current_deployment.has_value() && !current_deployment->completed())
		{
			return error(ERR_DATABASE);
		}

		if (current_deployment.has_value() && !database::combat_deployments::delete_deployment(player->get_id(), mission_id))
		{
			return error(ERR_DATABASE);
		}

		deployment_info.armored = get_arg("armored");
		deployment_info.battle_gear = get_arg("battle_gear");
		deployment_info.car = get_arg("car");
		deployment_info.combat_count = get_arg("combat_count");
		deployment_info.combat_rank_bottom = get_arg("combat_rank_bottom");
		deployment_info.combat_rank_top = get_arg("combat_rank_top");
		deployment_info.staff_power = get_arg("staff_power");
		deployment_info.sub_count = get_arg("sub_count");
		deployment_info.sub_rank_bottom = get_arg("sub_rank_bottom");
		deployment_info.sub_rank_top = get_arg("sub_rank_top");
		deployment_info.tank = get_arg("tank");
		deployment_info.team_id = get_arg("team_id");
		deployment_info.team_power = get_arg32("team_power");
		deployment_info.truck = get_arg("truck");
		deployment_info.walker_gear = get_arg("walker_gear");
		deployment_info.win_rate = get_arg("win_rate");

		const auto damage_seed = utils::cryptography::random::get_integer(1000u, 100000u);
		deployment_info.seed = damage_seed;
		deployment_info.dead_rate = static_cast<std::uint8_t>(utils::cryptography::random::get_integer(mission->min_dead_rate, mission->max_dead_rate));

		const auto time = mission->time * 60;
		const auto rand = utils::cryptography::random::get_integer(0u, 100u);
		const auto is_win = rand <= deployment_info.win_rate;

		if (!database::combat_deployments::deploy_mission(player->get_id(), mission_id, time, deployment_info, is_win))
		{
			return error(ERR_DATABASE);
		}

		return result;
	}

	bool cmd_deploy_mission::needs_player()
	{
		return true;
	}
}
