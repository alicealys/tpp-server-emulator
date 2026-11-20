#include <std_include.hpp>

#include "cmd_sync_emblem.hpp"

#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_sync_emblem::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		game::emblem_t emblem{};
		if (!emblem.parse(data["emblem"], true))
		{
			return error(ERR_INVALIDARG);
		}

		database::player_data::sync_emblem(player->get_id(), emblem);

		return result;
	}

	bool cmd_sync_emblem::needs_player()
	{
		return true;
	}
}
