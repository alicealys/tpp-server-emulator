#include <std_include.hpp>

#include "database/models/player_data.hpp"
#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_item_purchase.hpp"
#include "database/models/shop_purchases.hpp"

#include "cmd_purchase_mgo_item.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_purchase_mgo_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& purchase_id_j = data["purchase_id"];
		const auto& purchase_type_j = data["purchase_type"];
		const auto& price_j = data["purchase_type"];

		if (!purchase_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto purchase_id = purchase_id_j.get<std::uint32_t>();
		const auto info = database::mgo_item_purchase::get_purchase_info(purchase_id);

		if (!info.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		switch (info->purchase_id)
		{
		case database::mgo_item_purchase::character_slot:
		{
			const auto char_count = database::mgo_characters::get_character_count(player->get_id());
			if (char_count < database::mgo_characters::total_character_count && database::player_data::spend_mb_coins(player->get_id(), info->price))
			{
				database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::character_slot, info->price, static_cast<std::uint32_t>(char_count));
				database::mgo_item_purchase::purchase_item(player->get_id(), purchase_id);
				database::mgo_characters::create_character(player->get_id(), static_cast<std::uint32_t>(char_count));

				result["purchase_id"] = purchase_id;
				result["purchase_type"] = purchase_type_j;
				result["price"] = price_j;
				result["result_point"] = database::player_data::get_mb_coins(player->get_id());
			}
			else
			{
				return error(ERR_DATABASE);
			}
		}
		}

		return result;
	}
}
