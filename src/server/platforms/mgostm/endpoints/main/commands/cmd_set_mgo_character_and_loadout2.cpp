#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"

#include "cmd_set_mgo_character_and_loadout2.hpp"
#include "cmd_set_mgo_character2.hpp"
#include "cmd_set_mgo_loadout.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_character_and_loadout2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		if (!cmd_set_mgo_character2::set_character(data, player))
		{
			return error(ERR_INVALIDARG);
		}

		if (!cmd_set_mgo_loadout::set_loadout(data, player))
		{
			return error(ERR_INVALIDARG);
		}

		return {};
	}

	bool cmd_set_mgo_character_and_loadout2::needs_player()
	{
		return true;
	}
}
