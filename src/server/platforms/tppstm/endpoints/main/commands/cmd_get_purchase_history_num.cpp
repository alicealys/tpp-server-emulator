#include <std_include.hpp>

#include "database/models/shop_purchases.hpp"

#include "cmd_get_purchase_history_num.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_purchase_history_num::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["record_num"] = database::shop_purchases::get_history_size(player->get_id());

		return result;
	}
}
