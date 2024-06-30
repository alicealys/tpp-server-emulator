#include <std_include.hpp>

#include "cmd_create_player.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"
#include "database/models/player_data.hpp"
#include "database/models/event_rankings.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_create_player::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["xuid"] = {};

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		database::player_records::find_or_create(player->get_id());
		database::player_data::find_or_create(player->get_id());
		database::event_rankings::create_entries(player->get_id());

		result["player_id"] = player->get_id();
		result["result"] = "NOERR";

		return result;
	}
}
