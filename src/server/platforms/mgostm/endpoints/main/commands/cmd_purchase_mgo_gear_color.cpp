#include <std_include.hpp>

#include "database/models/player_data.hpp"
#include "database/models/mgo_data.hpp"
#include "database/models/mgo_color_purchases.hpp"
#include "database/models/shop_purchases.hpp"

#include "cmd_purchase_mgo_gear_color.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_purchase_mgo_gear_color::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& color_j = data["color"];
		const auto& gear_id_j = data["gear_id"];
		const auto& price_j = data["price"];
		const auto& purchase_type_j = data["purchase_type"];

		if (!color_j.is_number_unsigned() || !gear_id_j.is_number_unsigned() || !price_j.is_number_unsigned() || !purchase_type_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto target_color = color_j.get<std::uint32_t>();
		const auto gear_id = gear_id_j.get<std::uint32_t>();
		[[ maybe_unused ]] const auto price = price_j.get<std::uint32_t>();
		const auto purchase_type = purchase_type_j.get<std::uint32_t>();
		const auto gear_info = database::mgo_color_purchases::get_gear_info(gear_id);

		const auto buy_color = [&](const std::uint32_t type, const std::uint32_t point)
		{
			if (type == 3) // mb_coin
			{
				if (database::player_data::spend_mb_coins(player->get_id(), point))
				{
					database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::gears_color_variation, point, gear_id);
					database::mgo_color_purchases::buy_color(player->get_id(), database::mgo_color_purchases::gear, gear_id, target_color);
					return true;
				}
			}

			if (type == 2) // gp_coin
			{
				if (database::mgo_data::spend_gp_coins(player->get_id(), point))
				{
					database::mgo_color_purchases::buy_color(player->get_id(), database::mgo_color_purchases::gear, gear_id, target_color);
					return true;
				}
			}

			return false;
		};

		const auto do_purchase = [&]
		{
			if (database::mgo_color_purchases::has_gear(player->get_id(), gear_id) || gear_info.purchase_type == 0)
			{
				const auto color_opt = database::mgo_color_purchases::get_gear_color(target_color);
				if (!color_opt.has_value())
				{
					return false;
				}

				const auto purchased_colors = database::mgo_color_purchases::get_purchased_colors(player->get_id(), database::mgo_color_purchases::gear, gear_id);
				if (purchased_colors.contains(target_color))
				{
					return false;
				}

				return buy_color(color_opt->purchase_type, color_opt->point);
			}
			else
			{
				return buy_color(gear_info.purchase_type, gear_info.point);
			}
		};

		if (do_purchase())
		{
			result["color"] = target_color;
			result["gear_id"] = gear_id;
			result["price"] = gear_info.point;
			result["purchase_type"] = purchase_type;

			if (gear_info.purchase_type == 3)
			{
				result["result_point"] = database::player_data::get_mb_coins(player->get_id());
			}
			else
			{
				result["result_point"] = database::mgo_data::get_gp_coins(player->get_id());
			}
		}
		else
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}

		return result;
	}
}
