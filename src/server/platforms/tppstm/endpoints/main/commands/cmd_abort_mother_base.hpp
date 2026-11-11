#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_abort_mother_base final : public command_handler
	{
	public:
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		bool needs_player() override;

		static void send_abort(const database::players::player& player);

	private:
		static void send_interrupted_sneak_result(const database::players::player& player);

	};
}
