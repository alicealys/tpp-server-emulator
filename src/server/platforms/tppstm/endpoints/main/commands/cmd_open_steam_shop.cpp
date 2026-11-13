#include <std_include.hpp>

#include "cmd_open_steam_shop.hpp"
#include "cmd_get_steam_shop_item_list.hpp"

#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	cmd_open_steam_shop::cmd_open_steam_shop()
	{
		this->list_ = resource(RESOURCE_STEAM_SHOP_ITEM_LIST);
	}

	nlohmann::json cmd_open_steam_shop::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		auto& steam_item_id_j = data["steam_item_id"];

		nlohmann::json result;
		result["orderid"] = 0;
		result["transid"] = 0;
		result["result"] = game::get_error(ERR_DATABASE);

		if (!steam_item_id_j.is_number_unsigned())
		{
			return result;
		}

		const auto steam_item_id = steam_item_id_j.get<std::uint64_t>();
		if (steam_item_id > cmd_get_steam_shop_item_list::items.size())
		{
			return error(ERR_INVALIDARG);
		}
		
		const auto& item = cmd_get_steam_shop_item_list::items[steam_item_id];
		const auto player_data = database::player_data::find(player->get_id());

		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		player_data->give_reward(item.reward);

		return result;
	}

	bool cmd_open_steam_shop::needs_player()
	{
		return true;
	}
}
