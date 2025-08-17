#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"

#include "cmd_delete_mgo_character.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_delete_mgo_character::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& character_index_j = data["characterIndex"];

		if (!character_index_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto character_index = character_index_j.get<std::uint32_t>();
		if (character_index < 1 || character_index >= database::mgo_characters::total_character_count)
		{
			return error(ERR_INVALIDARG);
		}

		database::mgo_characters::delete_character(player->get_id(), character_index);

		return result;
	}

	bool cmd_delete_mgo_character::needs_player()
	{
		return true;
	}
}
