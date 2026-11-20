#include <std_include.hpp>

#include "cmd_consume_reserve.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_consume_reserve::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		
		return result;
	}

	bool cmd_consume_reserve::needs_player()
	{
		return true;
	}
}
