#include <std_include.hpp>

#include "database/models/player_data.hpp"
#include "database/models/mgo_data.hpp"
#include "database/models/mgo_color_purchase.hpp"

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

		const auto color = color_j.get<std::uint32_t>();
		const auto gear_id = gear_id_j.get<std::uint32_t>();
		[[ maybe_unused ]] const auto price = price_j.get<std::uint32_t>();
		const auto purchase_type = purchase_type_j.get<std::uint32_t>();
		const auto gear_info = database::mgo_color_purchase::get_gear_info(gear_id);

		const auto do_purchase = [&]
		{
			if (gear_info.purchase_type == 3) // mb_coin
			{
				if (database::player_data::spend_mb_coins(player->get_id(), gear_info.point))
				{
					database::mgo_color_purchase::buy_color(player->get_id(), gear_id, color);
					return true;
				}
			}

			if (gear_info.purchase_type == 2) // gp_coin
			{
				if (database::mgo_data::spend_gp_coins(player->get_id(), gear_info.point))
				{
					database::mgo_color_purchase::buy_color(player->get_id(), gear_id, color);
					return true;
				}
			}

			return false;
		};

		if (do_purchase())
		{
			result["color"] = color;
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
			return error(ERR_DATABASE);
		}

		return result;
	}
}
