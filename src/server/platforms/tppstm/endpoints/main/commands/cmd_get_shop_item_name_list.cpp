#include <std_include.hpp>

#include "cmd_get_shop_item_name_list.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_get_shop_item_name_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}