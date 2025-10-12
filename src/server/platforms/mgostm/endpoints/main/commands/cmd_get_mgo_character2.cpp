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

			const auto& avatar = characters[i].get_avatar();

			char_json["avatar"] = avatar.to_json();
			char_json["last_loadout"] = characters[i].get_last_loadout();
			char_json["name"] = characters[i].get_name();
			char_json["player_class"] = characters[i].get_player_class();
			char_json["player_type"] = characters[i].get_player_type();
		}

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());

		character["last_active"] = mgo_data->get_last_character_used();

		const auto& match_settings = mgo_data->get_match_settings();

		character["match"]["auto_leave"] = match_settings.auto_leave;
		character["match"]["briefing_time"] = match_settings.briefing_time;
		character["match"]["host_comment"] = match_settings.host_comment;
		character["match"]["max_capacity"] = std::clamp(match_settings.max_capacity, std::uint8_t(2u), std::uint8_t(16u));
		character["match"]["mission_slot_count"] = std::min(match_settings.mission_slot_count, std::uint8_t(5u));
		character["match"]["player_num"] = std::clamp(match_settings.player_num, std::uint8_t(2u), std::uint8_t(16u));

		for (auto i = 0; i < 5; i++)
		{
			character["match"]["mission_slot_list"][i]["flags"] = match_settings.mission_slots[i].flags;
			character["match"]["mission_slot_list"][i]["map"] = match_settings.mission_slots[i].map;
			character["match"]["mission_slot_list"][i]["night"] = match_settings.mission_slots[i].night;
			character["match"]["mission_slot_list"][i]["rule"] = match_settings.mission_slots[i].rule;
			character["match"]["mission_slot_list"][i]["rush"] = match_settings.mission_slots[i].rush;
			character["match"]["mission_slot_list"][i]["ticket"] = match_settings.mission_slots[i].ticket;
			character["match"]["mission_slot_list"][i]["time"] = match_settings.mission_slots[i].time;
			character["match"]["mission_slot_list"][i]["unique_character"] = match_settings.mission_slots[i].unique_character;
			character["match"]["mission_slot_list"][i]["weather"] = match_settings.mission_slots[i].weather;
		}

		const auto& preset_radio = mgo_data->get_preset_radio();

		character["preset_radio_rule_list"] = nlohmann::json::array();
		for (auto i = 0; i < 5; i++)
		{
			character["preset_radio_rule_list"][i]["preset_radio_id_list"] = nlohmann::json::array();

			for (auto o = 0; o < 8; o++)
			{
				character["preset_radio_rule_list"][i]["preset_radio_id_list"][o] = preset_radio.list[i].ids[o];
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
