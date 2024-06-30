#include <std_include.hpp>

#include "cmd_calc_cost_time_reduction.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_calc_cost_time_reduction::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}
		
		const auto& kind_j = data["kind"];
		const auto& remaining_time_j = data["remaining_time"];

		if (!kind_j.is_string() || !remaining_time_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		const auto kind = kind_j.get<std::string>();
		const auto remaining_time = remaining_time_j.get<std::uint32_t>();

		const auto get_cost = [&]
		{
			if (kind == "PLATFORM_CONSTRUCTION")
			{
				return utils::tpp::calculate_mb_coins(remaining_time, database::vars.cost_factor_platform_construction);
			}

			return utils::tpp::calculate_mb_coins(remaining_time, database::vars.cost_factor_generic);
		};

		result["cost"] = get_cost();

		return result;
	}
}
