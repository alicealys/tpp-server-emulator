#include <std_include.hpp>

#include "cmd_do_mgo_ascension.hpp"

#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_data.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_do_mgo_ascension::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& character_index_j = data["characterIndex"];
		if (!character_index_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}
		
		const auto character_index = character_index_j.get<std::uint32_t>();

		const auto characters = database::mgo_characters::get_character_list(player->get_id());
		if (character_index >= characters.size())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& character = characters[character_index];
		if (character.get_prestige() >= database::mgo_characters::max_prestige)
		{
			return error(ERR_INVALIDARG);
		}

		const auto next_prestige = character.get_prestige() + 1;

		const auto required_xp = database::mgo_characters::get_prestige_required_xp(next_prestige);
		const auto gp_bonus = database::mgo_characters::get_prestige_gp_bonus(next_prestige);

		if (character.get_xp() < required_xp)
		{
			return error(ERR_INVALIDARG);
		}

		database::mgo_characters::character_progression_params progression{};
		progression.xp = 0;
		progression.prestige = next_prestige;
		progression.legendary = character.get_legendary();

		if (!database::mgo_characters::update_character_progression(player->get_id(), character_index, progression))
		{
			return error(ERR_DATABASE);
		}

		result["characterIndex"] = character_index;
		result["bonusGp"] = gp_bonus;
		result["resultGp"] = database::mgo_data::add_gp_coins(player->get_id(), gp_bonus);

		return result;
	}
}
