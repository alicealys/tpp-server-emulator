#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_get_shop_item_name_list final : public command_handler
	{
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
