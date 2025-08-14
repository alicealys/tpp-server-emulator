#pragma once

#include "types/command_handler.hpp"

namespace emulator::mgo
{
	class cmd_get_mgo_purchasable_item_list final : public command_handler
	{
	public:
		cmd_get_mgo_purchasable_item_list();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	private:
		nlohmann::json list_;

	};
}
