#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_reqauth_https final : public command_handler
	{
	public:
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
		
	private:
		void wait_fail_delay(const std::chrono::system_clock::time_point start);
	};
}
