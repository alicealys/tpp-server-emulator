#include <std_include.hpp>

#include "target_list_nuclear.hpp"

namespace emulator::tpp
{
	target_list_t target_list_nuclear::generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto players = database::player_data::find_with_nukes(std::min(limit, 10u));

		for (const auto& id : players)
		{
			if (id == player.get_id())
			{
				continue;
			}

			target_data_t data{};
			data.player_id = id;
			targets.emplace_back(data);
		}

		return targets;
	}
}
