#include <std_include.hpp>

#include "cmd_purchase_online_development_completion.hpp"

#include "database/models/player_data.hpp"
#include "database/models/items.hpp"
#include "database/models/shop_purchases.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_purchase_online_development_completion::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& develop_id_j = data["develop_id"];

		if (!develop_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto item_id = develop_id_j.get<std::uint32_t>();

		nlohmann::json result;
		result["paid_coin"] = 0;

		const auto item = database::items::get_item(player->get_id(), item_id);
		if (item.get_develop() != database::items::indev)
		{
			result["error"] = game::get_error(ERR_ALREADY_COMPLETED);
			return result;
		}

		if (database::player_data::spend_mb_coins(player->get_id(), item.get_mb_coin()))
		{
			if (!database::items::force_develop(player->get_id(), item.get_id()))
			{
				database::player_data::add_mb_coins(player->get_id(), item.get_mb_coin());
				return error(ERR_DATABASE);
			}
			else
			{
				database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::time_reduction_development, item.get_mb_coin(), item.get_id());
			}
		}
		else
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}

		result["paid_coin"] = item.get_mb_coin();

		return result;
	}

	bool cmd_purchase_online_development_completion::needs_player()
	{
		return true;
	}
}
