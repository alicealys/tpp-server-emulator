#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"

#include "cmd_set_mgo_character_and_loadout2.hpp"
#include "cmd_set_mgo_character2.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_character_and_loadout2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!cmd_set_mgo_character2::set_character(data, player))
		{
			return error(ERR_INVALIDARG);
		}

		const auto char_count = database::mgo_characters::get_character_count(player->get_id());

		auto& loadout = data["loadout"];

		if (!loadout.is_object())
		{
			return error(ERR_INVALIDARG);
		}

		auto& loadout_character_list = loadout["character_list"];
		if (!loadout_character_list.is_array())
		{
			return error(ERR_INVALIDARG);
		}

		const auto update_count = std::min(char_count, loadout_character_list.size());

		for (auto i = 0ull; i < update_count; i++)
		{
			if (!loadout_character_list[i].is_object() || !loadout_character_list[i]["loadout_list"].is_array())
			{
				return error(ERR_INVALIDARG);
			}
		}

		for (auto i = 0ull; i < update_count; i++)
		{
			database::mgo_characters::update_character_loadouts(player->get_id(), static_cast<std::uint32_t>(i), loadout_character_list[i]["loadout_list"]);
		}

		return result;
	}

	bool cmd_set_mgo_character_and_loadout2::needs_player()
	{
		return true;
	}
}
