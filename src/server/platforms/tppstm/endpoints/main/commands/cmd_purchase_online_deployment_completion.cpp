#include <std_include.hpp>

#include "cmd_purchase_online_deployment_completion.hpp"

#include "database/models/combat_deployments.hpp"
#include "database/models/player_data.hpp"
#include "database/models/shop_purchases.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_purchase_online_deployment_completion::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& mission_id_j = data["mission_id"];
		if (!mission_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto mission_id = mission_id_j.get<std::uint32_t>();
		const auto deployment = database::combat_deployments::get_deployment(player->get_id(), mission_id);
		if (!deployment.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto time_left = static_cast<std::uint32_t>(deployment->get_time_left().count());
		if (time_left <= 0)
		{
			result["paid_coin"] = 0;
			return result;
		}

		const auto cost = game::calculate_mb_coins(time_left, database::vars.cost_factor_troops_completion);
		if (database::player_data::spend_mb_coins(player->get_id(), cost))
		{
			if (!database::combat_deployments::complete_deployment(player->get_id(), mission_id))
			{
				database::player_data::add_mb_coins(player->get_id(), cost);
				return error(ERR_DATABASE);
			}
			else
			{
				database::shop_purchases::add_spent(player->get_id(), database::shop_purchases::time_reduction_dispatch, time_left, cost, time_left);
			}
		}
		else
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}

		result["paid_coin"] = cost;

		return result;
	}
	
	bool cmd_purchase_online_deployment_completion::needs_player()
	{
		return true;
	}
}
