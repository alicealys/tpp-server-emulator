#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_get_steam_shop_item_list final : public command_handler
	{
	public:
		cmd_get_steam_shop_item_list();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	private:
		nlohmann::json list_;

	};
}
