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
			const auto loadout_count = characters[i].get_loadout_count();
			const auto loadouts = characters[i].get_loadouts();

			for (auto o = 0u; o < loadout_count; o++)
			{
				result["loadout"]["character_list"][i]["loadout_list"][o] = loadouts->loadouts[o].to_json();
			}
		}

		result["loadout"]["version"] = 192687032374006;

		return result;
	}

	bool cmd_get_mgo_loadout::needs_player()
	{
		return true;
	}
}
