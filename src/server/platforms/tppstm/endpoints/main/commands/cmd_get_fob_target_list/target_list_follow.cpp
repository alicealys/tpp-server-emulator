#include <std_include.hpp>

#include "target_list_follow.hpp"

#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	target_list_t target_list_follow::generate_impl(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto follows = database::player_follows::get_follows(player.get_id());
		const auto followers = database::player_follows::get_followers(player.get_id());

		for (const auto& follow : follows)
		{
			target_data_t data{};
			data.player_id = follow.get_player_id();
			data.player = follow.player;
			data.player_record = follow.record;
			data.extra_data["owner_detail_record"]["follower"] = followers.contains(follow.get_player_id());
			data.extra_data["owner_detail_record"]["follow"] = 1;
			targets.emplace_back(data);
		}

		return targets;
	}

	target_list_t target_list_follow::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		return generate_impl(player, player_data, limit);
	}
}
