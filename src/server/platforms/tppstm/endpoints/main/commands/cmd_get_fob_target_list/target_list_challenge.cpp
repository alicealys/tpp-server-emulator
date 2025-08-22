#include <std_include.hpp>

#include "target_list_challenge.hpp"

#include "database/models/player_records.hpp"
#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	target_list_t target_list_challenge::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		const auto follows = database::player_follows::get_follows(player.get_id());

		const auto list = database::players::find_with_security_challenge(std::min(limit, 10u));
		target_list_t targets;

		if (player.is_security_challenge_enabled())
		{
			target_data_t target{};
			target.player_id = player.get_id();
			targets.emplace_back(target);
		}

		for (auto& row : list)
		{
			if (row.get_id() != player.get_id() && !follows.contains(row.get_id()))
			{
				target_data_t target{};
				target.player_id = row.get_id();
				targets.emplace_back(target);
			}
		}

		return targets;
	}
}
