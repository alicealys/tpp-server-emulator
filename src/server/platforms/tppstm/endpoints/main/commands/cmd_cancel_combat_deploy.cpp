#include <std_include.hpp>

#include "cmd_cancel_combat_deploy.hpp"

#include "database/models/combat_deployments.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_cancel_combat_deploy::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		database::combat_deployments::delete_all_deployments(player->get_id());

		return result;
	}

	bool cmd_cancel_combat_deploy::needs_player()
	{
		return true;
	}
}
