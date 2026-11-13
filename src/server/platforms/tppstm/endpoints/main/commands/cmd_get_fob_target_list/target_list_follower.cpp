#include <std_include.hpp>

#include "target_list_follower.hpp"

#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	target_list_t target_list_follower::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto follows = database::player_follows::get_follows(player.get_id());
		const auto followers = database::player_follows::get_followers(player.get_id());

		for (const auto& follower : followers)
		{
			target_data_t data{};
			data.player_id = follower.get_player_id();
			data.player = follower.player;
			data.player_record = follower.record;
			data.extra_data["owner_detail_record"]["follower"] = 1;
			data.extra_data["owner_detail_record"]["follow"] = follows.contains(follower.get_player_id());
			targets.emplace_back(data);
		}

		return targets;
	}
}
