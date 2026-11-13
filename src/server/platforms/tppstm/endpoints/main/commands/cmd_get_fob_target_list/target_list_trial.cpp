#include <std_include.hpp>

#include "target_list_trial.hpp"

#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	target_list_t target_list_trial::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto follows = database::player_follows::get_follows(player.get_id());
		const auto followers = database::player_follows::get_followers(player.get_id());

		target_data_t self{};
		self.player_id = player.get_id();
		targets.emplace_back(self);

		for (const auto& follow : follows)
		{
			target_data_t data{};
			data.player_id = follow.player.get_id();
			data.player = follow.player;
			data.player_record = follow.record;
			data.extra_data["owner_detail_record"]["follower"] = followers.contains(follow.player.get_id());
			data.extra_data["owner_detail_record"]["follow"] = 1;
			targets.emplace_back(data);
		}

		return targets;
	}
}
