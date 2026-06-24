#include <std_include.hpp>

#include "cmd_purchase_mgo_weapon_color.hpp"

#include "database/models/mgo_color_purchases.hpp"
#include "database/models/mgo_data.hpp"
#include "database/models/player_data.hpp"
#include "database/models/shop_purchases.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_purchase_mgo_weapon_color::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& color_j = data["color"];
		const auto& weapon_id_j = data["weapon_id"];
		const auto& price_j = data["price"];
		const auto& purchase_type_j = data["purchase_type"];

		if (!color_j.is_number_unsigned() || !weapon_id_j.is_number_unsigned() || !price_j.is_number_unsigned() || !purchase_type_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto target_color = color_j.get<std::uint32_t>();
		const auto weapon_id = weapon_id_j.get<std::uint32_t>();
		[[ maybe_unused ]] const auto price = price_j.get<std::uint32_t>();
		const auto purchase_type = purchase_type_j.get<std::uint32_t>();

		const auto color_info = database::mgo_color_purchases::get_weapon_color(target_color);
		const auto purchased_colors = database::mgo_color_purchases::get_purchased_colors(player->get_id(), database::mgo_color_purchases::weapon, weapon_id);

		if (!color_info.has_value() || purchased_colors.contains(target_color))
		{
			return error(ERR_INVALIDARG);
		}

		const auto buy_color = [&]
		{
			if (color_info->purchase_type == 3) // mb_coin
			{
				if (database::player_data::spend_mb_coins(player->get_id(), color_info->point))
				{
					database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::weapons_color_variation, color_info->point, weapon_id);
					database::mgo_color_purchases::buy_color(player->get_id(), database::mgo_color_purchases::weapon, weapon_id, color_info->color);
					return true;
				}
			}

			if (color_info->purchase_type == 2) // gp_coin
			{
				if (database::mgo_data::spend_gp_coins(player->get_id(), color_info->point))
				{
					database::mgo_color_purchases::buy_color(player->get_id(), database::mgo_color_purchases::weapon, weapon_id, color_info->color);
					return true;
				}
			}

			return false;
		};

		if (buy_color())
		{
			result["color"] = target_color;
			result["gear_id"] = weapon_id;
			result["price"] = color_info->point;
			result["purchase_type"] = purchase_type;
			result["error_code"] = 0;

			if (color_info->purchase_type == 3)
			{
				result["result_point"] = database::player_data::get_mb_coins(player->get_id());
			}
			else
			{
				result["result_point"] = database::mgo_data::get_gp_coins(player->get_id());
			}

			return result;
		}
		else
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}
	}

	bool cmd_purchase_mgo_weapon_color::needs_player()
	{
		return true;
	}
}
