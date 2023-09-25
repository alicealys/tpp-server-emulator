#include <std_include.hpp>

#include "cmd_abort_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_abort_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		database::players::abort_mother_base(player->get_id());

		return result;
	}
}
