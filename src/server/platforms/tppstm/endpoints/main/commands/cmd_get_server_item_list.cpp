#include <std_include.hpp>

#include "cmd_get_server_item_list.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_server_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		auto status_list = database::items::get_item_list(player->get_id());

		auto index = 0;
		for (const auto& item_status : status_list)
		{
			const auto i = index++;

			result["item_list"][i]["create_date"] = item_status.second.get_create_time();
			result["item_list"][i]["develop"] = item_status.second.get_develop();
			result["item_list"][i]["gmp"] = item_status.second.get_gmp();
			result["item_list"][i]["id"] = item_status.second.get_id();
			result["item_list"][i]["left_second"] = item_status.second.get_left_second();
			result["item_list"][i]["max_second"] = item_status.second.get_max_second();
			result["item_list"][i]["mb_coin"] = utils::tpp::calculate_mb_coins(item_status.second.get_max_second());
			result["item_list"][i]["open"] = item_status.second.is_open();
		}

		result["item_num"] = status_list.size();
		result["develop_limit"] = database::vars.item_dev_limit;

		result["result"] = "NOERR";
		result["xuid"] = {};

		return result;
	}
}
