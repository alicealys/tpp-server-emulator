#include <std_include.hpp>

#include "cmd_get_mbcoin_remainder.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_mbcoin_remainder::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		result["remainder"] = database::player_data::get_mb_coins(player->get_id());
		result["result"] = utils::tpp::get_error(NOERR);

		return result;
	}
}
