#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"

#include "cmd_get_mgo_loadout.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_loadout::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto characters = database::mgo_characters::get_character_list(player->get_id());

		result["loadout"]["character_list"] = nlohmann::json::array();
		for (auto i = 0ull; i < characters.size(); i++)
		{
			result["loadout"]["character_list"][i]["loadout_list"] = characters[i].get_loadouts();
		}

		result["loadout"]["version"] = 130186347867651;

		return result;
	}

	bool cmd_get_mgo_loadout::needs_player()
	{
		return true;
	}
}
