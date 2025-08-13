#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"

#include "cmd_get_mgo_character2.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_character2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& character = result["character"];

		const auto characters = database::mgo_characters::get_character_list(player->get_id());

		character["character_list"] = nlohmann::json::array();
		for (auto i = 0ull; i < characters.size(); i++)
		{
			auto& char_json = character["character_list"][i];
			char_json["avatar"] = characters[i].get_avatar();
			char_json["last_loadout"] = characters[i].get_last_loadout();
			char_json["player_name"] = characters[i].get_name();
			char_json["player_class"] = characters[i].get_player_class();
			char_json["player_type"] = characters[i].get_player_type();
		}

		character["last_active"] = 0;

		character["match"]["auto_leave"] = 0;
		character["match"]["briefing_time"] = 0;
		character["match"]["host_comment"] = 0;
		character["match"]["max_capacity"] = 0;
		character["match"]["mission_slot_count"] = 0;
		character["match"]["mission_slot_list"] = nlohmann::json::array();
		character["match"]["player_num"] = 0;

		character["preset_radio_rule_list"] = nlohmann::json::array();
		for (auto i = 0; i < 5; i++)
		{
			character["preset_radio_rule_list"][i]["preset_radio_id_list"] = nlohmann::json::array();

			for (auto o = 0; o < 8; o++)
			{
				character["preset_radio_rule_list"][i]["preset_radio_id_list"][o] = 0;
			}
		}

		character["selected_bgm"] = 0;
		character["version"] = 0;

		return result;
	}

	bool cmd_get_mgo_character2::needs_player()
	{
		return true;
	}
}
