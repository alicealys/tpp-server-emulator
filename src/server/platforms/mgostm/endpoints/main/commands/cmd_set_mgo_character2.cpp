#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"

#include "cmd_set_mgo_character2.hpp"

namespace emulator::mgo
{
	bool cmd_set_mgo_character2::set_character(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto char_count = database::mgo_characters::get_character_count(player->get_id());
		auto& character = data["character"];

		if (!character.is_object())
		{
			return false;
		}

		auto& character_list = character["character_list"];
		auto& permanent_unlock = data["permanent_unlock"];

		if (!character_list.is_array())
		{
			return false;
		}

		const auto update_count = std::min(char_count, character_list.size());

		for (auto i = 0ull; i < update_count; i++)
		{
			if (!character_list[i].is_object() || !character_list[i]["avatar"].is_object() ||
				!character_list[i]["last_loadout"].is_number_unsigned() || !character_list[i]["player_type"].is_number_unsigned() ||
				!character_list[i]["player_class"].is_number_unsigned() || !character_list[i]["name"].is_string())
			{
				return false;
			}
		}

		for (auto i = 0ull; i < update_count; i++)
		{
			database::mgo_characters::character_params params{};
			params.avatar = character_list[i]["avatar"].dump();
			params.last_loadout = character_list[i]["last_loadout"].get<std::uint32_t>();
			params.player_type = character_list[i]["player_type"].get<std::uint32_t>();
			params.player_class = character_list[i]["player_class"].get<std::uint32_t>();
			params.name = character_list[i]["name"].get<std::string>();

			const auto get_permanent_unlock = [&]
			{
				if (!permanent_unlock.is_array())
				{
					return 0u;
				}

				if (!permanent_unlock[i].is_number_unsigned())
				{
					return 0u;
				}

				return permanent_unlock[i].get<std::uint32_t>();
			};

			params.permanent_unlock = get_permanent_unlock();

			database::mgo_characters::update_character(player->get_id(), static_cast<std::uint32_t>(i), params);
		}

		database::mgo_data::data_params params{};

		auto& last_active = character["last_active"];
		auto& selected_bgm = character["selected_bgm"];

		if (last_active.is_number_unsigned() && selected_bgm.is_number_unsigned())
		{
			params.bgm_selected = selected_bgm.get<std::uint32_t>();
			params.last_character_used = last_active.get<std::uint32_t>();

		}

		auto& match = character["match"];
		if (match.is_object() && match["mission_slot_list"].is_array() && match["mission_slot_list"].size() == 5)
		{
			params.match_settings.auto_leave = match["auto_leave"].get<std::uint8_t>();
			params.match_settings.briefing_time = match["briefing_time"].get<std::uint8_t>();
			params.match_settings.host_comment = match["host_comment"].get<std::uint32_t>();
			params.match_settings.max_capacity = match["max_capacity"].get<std::uint8_t>();
			params.match_settings.mission_slot_count = std::min(match["mission_slot_count"].get<std::uint8_t>(), std::uint8_t(5));
			params.match_settings.player_num = match["player_num"].get<std::uint8_t>();

			for (auto i = 0u; i < params.match_settings.mission_slot_count; i++)
			{
				params.match_settings.mission_slots[i].flags = match["mission_slot_list"][i]["flags"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].map = match["mission_slot_list"][i]["map"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].night = match["mission_slot_list"][i]["night"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].rule = match["mission_slot_list"][i]["rule"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].rush = match["mission_slot_list"][i]["rush"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].ticket = match["mission_slot_list"][i]["ticket"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].time = match["mission_slot_list"][i]["time"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].unique_character = match["mission_slot_list"][i]["unique_character"].get<std::uint8_t>();
				params.match_settings.mission_slots[i].weather = match["mission_slot_list"][i]["weather"].get<std::uint8_t>();
			}
		}

		auto& preset_radio_rule_list = character["preset_radio_rule_list"];
		if (preset_radio_rule_list.is_array() && preset_radio_rule_list.size() == 5)
		{
			for (auto i = 0; i < 5; i++)
			{
				if (preset_radio_rule_list[i].is_object() && preset_radio_rule_list[i]["preset_radio_id_list"].is_array() &&
					preset_radio_rule_list[i]["preset_radio_id_list"].size() == 8)
				{
					for (auto o = 0; o < 8; o++)
					{
						params.preset_radio.list[i].ids[o] = preset_radio_rule_list[i]["preset_radio_id_list"][o].get<std::uint8_t>();
					}
				}
			}
		}

		database::mgo_data::set_values_from_character(player->get_id(), params);

		return true;
	}

	nlohmann::json cmd_set_mgo_character2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		if (!this->set_character(data, player))
		{
			return error(ERR_INVALIDARG);
		}

		return {};
	}

	bool cmd_set_mgo_character2::needs_player()
	{
		return true;
	}
}
