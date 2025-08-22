#pragma once

#include "base_list.hpp"

namespace emulator::tpp
{
	class target_list_enemy final : public base_list
	{
	public:
		target_list_t generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit) override;
	};
}
