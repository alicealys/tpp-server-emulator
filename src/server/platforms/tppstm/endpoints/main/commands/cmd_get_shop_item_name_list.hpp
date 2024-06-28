#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_get_shop_item_name_list : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
