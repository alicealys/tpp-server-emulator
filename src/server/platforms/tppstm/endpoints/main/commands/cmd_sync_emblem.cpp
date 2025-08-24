#include <std_include.hpp>

#include "cmd_sync_emblem.hpp"

#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_sync_emblem::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		if (!data["emblem"].is_object())
		{
			return error(ERR_INVALIDARG);
		}

		game::emblem_t emblem{};
		game::parse_emblem(data["emblem"], emblem);

		database::player_data::sync_emblem(player->get_id(), emblem);

		return result;
	}
}
