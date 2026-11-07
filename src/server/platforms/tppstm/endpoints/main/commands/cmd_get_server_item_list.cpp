#include <std_include.hpp>

#include "cmd_get_server_item_list.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_server_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto item_list = database::items::get_item_list(player->get_id());
		for (auto i = 0ull; i < item_list.size(); i++)
		{
			item_list[i].to_json(result["item_list"][i]);
		}

		result["item_num"] = item_list.size();
		result["develop_limit"] = database::vars.item_dev_limit;

		return result;
	}

	bool cmd_get_server_item_list::needs_player()
	{
		return true;
	}
}
