#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"

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
			char_json["name"] = characters[i].get_name();
			char_json["player_class"] = characters[i].get_player_class();
			char_json["player_type"] = characters[i].get_player_type();
		}

		if (characters.empty())
		{
			for (auto i = 0ull; i < database::mgo_characters::max_character_count; i++)
			{
				static const auto base_avatar = utils::resources::load_json(RESOURCE_MGO_BASE_AVATAR);
				auto& char_json = character["character_list"][i];
				char_json["avatar"] = base_avatar;
				char_json["last_loadout"] = 0;
				char_json["name"] = "";
				char_json["player_class"] = 0;
				char_json["player_type"] = 0;
			}
		}

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());

		character["last_active"] = mgo_data->get_last_character_used();

		character["match"]["auto_leave"] = mgo_data->get_match_auto_leave();
		character["match"]["briefing_time"] = mgo_data->get_match_briefing_time();
		character["match"]["host_comment"] = mgo_data->get_match_host_comment();
		character["match"]["max_capacity"] = mgo_data->get_match_max_capacity();
		character["match"]["mission_slot_count"] = 0;
		character["match"]["mission_slot_list"] = nlohmann::json::array();
		character["match"]["player_num"] = mgo_data->get_mission_player_num();

		character["preset_radio_rule_list"] = nlohmann::json::array();
		for (auto i = 0; i < 5; i++)
		{
			character["preset_radio_rule_list"][i]["preset_radio_id_list"] = nlohmann::json::array();

			for (auto o = 0; o < 8; o++)
			{
				character["preset_radio_rule_list"][i]["preset_radio_id_list"][o] = 0;
			}
		}

		character["selected_bgm"] = mgo_data->get_bgm_selected();
		character["version"] = 130186347867651;

		return result;
	}

	bool cmd_get_mgo_character2::needs_player()
	{
		return true;
	}
}
