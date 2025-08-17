#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"

#include "cmd_set_mgo_character_and_loadout2.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_character_and_loadout2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto char_count = database::mgo_characters::get_character_count(player->get_id());
		if (char_count < database::mgo_characters::initial_character_count)
		{
			const auto needs = database::mgo_characters::initial_character_count - char_count;
			for (auto i = 0ull; i < needs; i++)
			{
				database::mgo_characters::create_character(player->get_id(), static_cast<std::uint32_t>(char_count + i));
			}
		}

		auto& character = data["character"];
		auto& loadout = data["loadout"];

		if (!character.is_object() || !loadout.is_object())
		{
			return error(ERR_INVALIDARG);
		}

		auto& character_list = character["character_list"];
		auto& loadout_character_list = loadout["character_list"];
		auto& permanent_unlock = data["permanent_unlock"];

		if (!loadout_character_list.is_array() || !character_list.is_array())
		{
			return error(ERR_INVALIDARG);
		}

		if (character_list.size() != loadout_character_list.size())
		{
			return error(ERR_INVALIDARG);
		}

		const auto update_count = std::min(char_count, character_list.size());

		for (auto i = 0ull; i < update_count; i++)
		{
			if (!character_list[i].is_object() || !character_list[i]["avatar"].is_object() ||
				!character_list[i]["last_loadout"].is_number_unsigned() || !character_list[i]["player_type"].is_number_unsigned() ||
				!character_list[i]["player_class"].is_number_unsigned() || !character_list[i]["name"].is_string() ||
				!loadout_character_list[i].is_object() || !loadout_character_list[i]["loadout_list"].is_array())
			{
				return error(ERR_INVALIDARG);
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
			params.loadouts = loadout_character_list[i]["loadout_list"].dump();

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

		auto& last_active = character["last_active"];
		auto& selected_bgm = character["selected_bgm"];

		if (last_active.is_number_unsigned() && selected_bgm.is_number_unsigned())
		{
			database::mgo_data::character_params params{};
			params.bgm_selected = selected_bgm.get<std::uint32_t>();
			params.last_character_used = last_active.get<std::uint32_t>();

			database::mgo_data::set_values_from_character(player->get_id(), params);
		}

		return result;
	}

	bool cmd_set_mgo_character_and_loadout2::needs_player()
	{
		return true;
	}
}
