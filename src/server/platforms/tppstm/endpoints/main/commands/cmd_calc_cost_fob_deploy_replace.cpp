#include <std_include.hpp>

#include "cmd_calc_cost_fob_deploy_replace.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_calc_cost_fob_deploy_replace::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		database::fob_deploy_assists::team_info_t team_info{};
		if (!team_info.parse(data["team_info"]))
		{
			return error(ERR_INVALIDARG);
		}

		result["cost"] = team_info.calc_replace_cost();
		return result;
	}
}
