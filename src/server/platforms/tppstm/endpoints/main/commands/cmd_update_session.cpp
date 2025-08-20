#include <std_include.hpp>

#include "cmd_update_session.hpp"

#include "database/models/players.hpp"
#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_update_session::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		if (!database::players::update_session(player.value()))
		{
			return error(ERR_DATABASE);
		}

		result["fob_index"] = -1;
		result["sneak_mode"] = -1;

		{
			const auto active_sneak = database::players::find_active_sneak(player->get_id(), true, true);
			if (active_sneak.has_value())
			{
				result["fob_index"] = 0;
				result["sneak_mode"] = 0;
				return result;
			}
		}

		{
			const auto follows = database::player_follows::get_follows(player->get_id());
			for (const auto& follow_id : follows)
			{
				const auto active_sneak = database::players::find_active_sneak(follow_id, true, true);
				if (active_sneak.has_value())
				{
					result["fob_index"] = 0;
					result["sneak_mode"] = 0;
					return result;
				}
			}
		}

		return result;
	}
}
