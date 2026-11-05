#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_get_pf_point_exchange_params final : public command_handler
	{
	public:
		static nlohmann::json generate(const std::optional<database::players::player>& player, const bool is_event);
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	};
}
