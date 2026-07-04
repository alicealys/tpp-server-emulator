#include <std_include.hpp>

#include "cmd_get_security_info.hpp"
#include "cmd_get_security_product_list.hpp"

#include "database/models/shop_purchases.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_security_info::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["end_date"] = 0;
		result["in_contract"] = 0;
		result["in_interval"] = 0;
		result["interval_end_date"] = 0;

		const auto last_purchase = database::shop_purchases::get_last_item_purchase_range(player->get_id(),
			database::shop_purchases::fob_defense_insurance_beg, database::shop_purchases::fob_defense_insurance_end);

		if (!last_purchase.has_value())
		{
			return result;
		}

		const auto product = cmd_get_security_product_list::get_security_product(last_purchase->get_item_type());
		if (!product.has_value())
		{
			return result;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto end_date = last_purchase->get_date() + product->period_hour * 1h;
		const auto interval_end_date = last_purchase->get_date() + product->interval_hour * 1h;

		if (now >= end_date)
		{
			return result;
		}

		result["end_date"] = end_date.count();
		result["in_contract"] = 1;
		result["in_interval"] = now < interval_end_date;
		result["interval_end_date"] = interval_end_date.count();

		return result;
	}

	bool cmd_get_security_info::needs_player()
	{
		return true;
	}
}
