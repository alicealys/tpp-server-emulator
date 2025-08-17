#include <std_include.hpp>

#include "database/models/players.hpp"
#include "database/models/mgo_characters.hpp"

#include "cmd_get_mgo_global_progression.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_global_progression::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& target = data["target"];
		if (!target.is_object())
		{
			return error(ERR_INVALIDARG);
		}

		const auto target_player = get_target_player(target);
		if (!target_player.has_value())
		{
			for (auto i = 0ull; i < database::mgo_characters::initial_character_count; i++)
			{
				result["global_progression"][i]["player_class"] = 0;
				result["global_progression"][i]["prestige"] = 0;
				result["global_progression"][i]["xp"] = 0;
			}

			return result;
		}

		result["global_progression"] = nlohmann::json::array();

		const auto characters = database::mgo_characters::get_character_list(target_player->get_id());
		for (auto i = 0ull; i < characters.size(); i++)
		{
			result["global_progression"][i]["player_class"] = characters[i].get_player_class();
			result["global_progression"][i]["prestige"] = characters[i].get_prestige();
			result["global_progression"][i]["xp"] = characters[i].get_xp();
		}

		return result;
	}
}
