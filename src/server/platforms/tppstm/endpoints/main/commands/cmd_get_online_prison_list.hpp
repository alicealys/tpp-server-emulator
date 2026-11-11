#pragma once

#include "types/command_handler.hpp"

namespace emulator::tpp
{
	class cmd_get_online_prison_list final : public command_handler
	{
	public:
		nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player) override;
	
	private:
		nlohmann::json get_free_list(const database::players::player& player, const std::uint32_t offset, const std::uint32_t num);
		nlohmann::json get_captive_list(const database::players::player& player, const std::uint32_t offset, const std::uint32_t num);

	};
}
