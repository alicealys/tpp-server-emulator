#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"

#include "cmd_get_mgo_progression.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_progression::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto characters = database::mgo_characters::get_character_list(player->get_id());

		auto& progression = result["progression"];

		progression["character_list"] = nlohmann::json::array();
		progression["permanent_unlock_list"] = nlohmann::json::array();

		for (auto i = 0ull; i < characters.size(); i++)
		{
			progression["character_list"][i]["legendary"] = characters[i].get_legendary();
			progression["character_list"][i]["prestige"] = characters[i].get_legendary();
			progression["character_list"][i]["xp"] = characters[i].get_xp();

			const auto list = characters[i].get_permanent_unlock_list();
			if (list != 0)
			{
				progression["permanent_unlock_list"][i] = list;
			}
		}

		progression["version"] = 143737279559449;

		return result;
	}
}
