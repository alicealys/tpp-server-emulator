#include <std_include.hpp>

#include "cmd_open_steam_shop.hpp"

#include "database/models/player_data.hpp"

namespace tpp
{
	cmd_open_steam_shop::cmd_open_steam_shop()
	{
		this->list_ = resource(RESOURCE_STEAM_SHOP_ITEM_LIST);
	}

	nlohmann::json cmd_open_steam_shop::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& steam_item_id_j = data["steam_item_id"];

		nlohmann::json result;
		result["orderid"] = 0;
		result["transid"] = 0;
		result["result"] = utils::tpp::get_error(ERR_INVALIDARG); // intentional error

		if (!steam_item_id_j.is_number_unsigned())
		{
			return result;
		}

		const auto steam_item_id = steam_item_id_j.get<std::uint64_t>();
		const auto& list = this->list_["list"];

		if (!list.is_array())
		{
			return result;
		}

		const auto get_item_index = [&]
		{
			for (auto i = 0; i < list.size(); i++)
			{
				if (list[i]["steam_item_id"] == steam_item_id)
				{
					return i;
				}
			}

			return -1;
		};

		const auto index = get_item_index();
		if (index == -1)
		{
			return result;
		}

		const auto& mb_coins_j = list[index]["mb_coins"];
		if (!mb_coins_j.is_number_unsigned())
		{
			return result;
		}

		const auto mb_coins = mb_coins_j.get<std::int32_t>();
		database::player_data::add_coins(player->get_id(), mb_coins);

		return result;
	}

	bool cmd_open_steam_shop::needs_player()
	{
		return true;
	}
}
