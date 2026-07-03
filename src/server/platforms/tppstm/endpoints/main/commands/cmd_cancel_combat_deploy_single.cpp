#include <std_include.hpp>

#include "cmd_cancel_combat_deploy_single.hpp"

#include "database/models/combat_deployments.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_cancel_combat_deploy_single::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& team_id_j = data["team_id"];

		if (!team_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto team_id = team_id_j.get<std::uint32_t>();

		const auto deployments = database::combat_deployments::get_deployments(player->get_id());
		for (auto& deployment : deployments)
		{
			if (deployment.get_deployment_info().team_id == team_id)
			{
				database::combat_deployments::delete_deployment(player->get_id(), deployment.get_mission_id());
			}
		}

		return result;
	}

	bool cmd_cancel_combat_deploy_single::needs_player()
	{
		return true;
	}
}
