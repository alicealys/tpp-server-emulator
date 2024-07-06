#pragma once

#include "database/models/event_rankings.hpp"
#include "database/models/fobs.hpp"
#include "database/models/items.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_follows.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/wormholes.hpp"

#pragma warning(push)
#pragma warning(disable: 4702)

#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 0
#include <sol/sol.hpp>

namespace scripting::engine
{
	std::optional<nlohmann::json> execute_command_hook(const std::string& command, nlohmann::json& data, const std::optional<database::players::player>& player);
}
