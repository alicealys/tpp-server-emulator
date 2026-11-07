#include <std_include.hpp>

#include "cmd_set_currentplayer.hpp"
#include "cmd_get_daily_reward.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_set_currentplayer::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		cmd_get_daily_reward::give_daily_reward(player.value());
		result["player_id"] = player->get_id();

		return result;
	}

	bool cmd_set_currentplayer::needs_player()
	{
		return true;
	}
}
