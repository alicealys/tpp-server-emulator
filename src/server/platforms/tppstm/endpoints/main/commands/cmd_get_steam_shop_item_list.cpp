#include <std_include.hpp>

#include "cmd_get_steam_shop_item_list.hpp"

namespace tpp
{
	cmd_get_steam_shop_item_list::cmd_get_steam_shop_item_list()
	{
		this->list_ = resource(RESOURCE_STEAM_SHOP_ITEM_LIST);
	}

	nlohmann::json cmd_get_steam_shop_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
