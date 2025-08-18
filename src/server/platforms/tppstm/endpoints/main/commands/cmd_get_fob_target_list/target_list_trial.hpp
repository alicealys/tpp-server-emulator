#pragma once

#include "base_list.hpp"

namespace emulator::tpp
{
	class target_list_trial final : public base_list
	{
	public:
		target_list_t generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit) override;
	};
}
