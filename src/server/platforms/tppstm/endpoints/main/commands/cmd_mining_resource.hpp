#pragma once

#include "types/command_handler.hpp"

namespace tpp
{
	class cmd_mining_resource : public command_handler
	{
	public:
		cmd_mining_resource();
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;

	private:
		nlohmann::json list_;

	};
}
