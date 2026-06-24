#include <std_include.hpp>

#include "cmd_get_mgo_purchasable_weapon_color.hpp"

#include "database/models/mgo_color_purchases.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_purchasable_weapon_color::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& weapon_id_j = data["weapon_id"];
		if (!weapon_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto weapon_id = weapon_id_j.get<std::uint32_t>();

		result["purchasable_weapon_color"]["already_released"] = 1;
		result["purchasable_weapon_color"]["release_date"] = 0;
		result["purchasable_weapon_color"]["weapon_id"] = weapon_id;
		result["purchasable_weapon_color"]["purchasable_color_list"] = nlohmann::json::array();

		auto& color_list = result["purchasable_weapon_color"]["purchasable_color_list"];

		const auto& colors = database::mgo_color_purchases::get_weapon_colors();
		const auto purchased_colors = database::mgo_color_purchases::get_purchased_colors(player->get_id(), database::mgo_color_purchases::weapon, weapon_id);

		for (auto i = 0ull; i < colors.size(); i++)
		{
			color_list[i]["already_purchased"] = 0;
			color_list[i]["color"] = colors[i].color;
			color_list[i]["level"] = colors[i].level;
			color_list[i]["point"] = colors[i].point;
			color_list[i]["prestige"] = colors[i].prestige;
			color_list[i]["purchase_type"] = colors[i].purchase_type;
		}

		for (auto i = 0ull; i < colors.size(); i++)
		{
			const auto color_id = color_list[i]["color"].get<std::uint32_t>();
			color_list[i]["already_purchased"] = purchased_colors.contains(color_id) ? 1 : 0;
		}

		return result;
	}

	bool cmd_get_mgo_purchasable_weapon_color::needs_player()
	{
		return true;
	}
}
