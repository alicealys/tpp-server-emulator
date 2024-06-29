#include <std_include.hpp>

#include "cmd_get_steam_shop_item_list.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_get_steam_shop_item_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
