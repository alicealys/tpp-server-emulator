#include <std_include.hpp>

#include "database/models/shop_purchases.hpp"

#include "cmd_get_purchase_history.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_purchase_history::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& num_j = data["num"];

		if (!num_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto num = num_j.get<std::uint32_t>();
		const auto purchase_history = database::shop_purchases::get_history(player->get_id(), num);

		for (auto i = 0ull; i < purchase_history.size(); i++)
		{
			result["list"][i]["coin_quantity"] = purchase_history[i].get_coin_quantity();
			result["list"][i]["date"] = purchase_history[i].get_date().count();
			result["list"][i]["event_type"] = purchase_history[i].get_event_type();
			result["list"][i]["expire_date"] = purchase_history[i].get_expire_date().count();
			result["list"][i]["item_quantity"] = purchase_history[i].get_item_quantity();
			result["list"][i]["item_type"] = purchase_history[i].get_item_type();
			result["list"][i]["param1"] = purchase_history[i].get_param1();
			result["list"][i]["remaining_coin"] = purchase_history[i].get_remaining_coin();
		}

		return result;
	}
}
