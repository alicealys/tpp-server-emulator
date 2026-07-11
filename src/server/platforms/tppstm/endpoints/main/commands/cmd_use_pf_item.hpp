#pragma once

#include "types/command_handler.hpp"

#include "database/models/pf_league.hpp"

namespace emulator::tpp
{
	class cmd_use_pf_item final : public command_handler
	{
	public:
		static nlohmann::json generate(nlohmann::json& data,
			const std::optional<database::players::player>& player,
			const std::optional<database::pf_league::pf_league>& pf_league);

		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;
	};
}
