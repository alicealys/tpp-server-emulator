#include <std_include.hpp>

#include "cmd_purchase_platform_construction.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_purchase_platform_construction::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& remaining_time_j = data["remaining_time"];

		if (!remaining_time_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto remaining_time = remaining_time_j.get<std::uint32_t>();
		const auto cost = utils::tpp::calculate_mb_coins(remaining_time, database::vars.cost_factor_platform_construction);
		if (!database::player_data::spend_coins(player->get_id(), cost))
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}
		
		return error(NOERR);
	}

	bool cmd_purchase_platform_construction::needs_player()
	{
		return true;
	}
}
