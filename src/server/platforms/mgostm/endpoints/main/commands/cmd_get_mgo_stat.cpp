#include <std_include.hpp>

#include "database/models/mgo_stats.hpp"

#include "cmd_get_mgo_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		if (!data["target"].is_object())
		{
			return error(ERR_INVALIDARG);
		}

		const auto target_player_opt = get_target_player(data["target"]);
		const auto target_player = target_player_opt.value_or(player.value());

		nlohmann::json result;
		
		const auto stats = database::mgo_stats::get_stats(target_player.get_id());
		result["stat"] = stats->to_json();

		return result;
	}

	bool cmd_get_mgo_stat::needs_player()
	{
		return true;
	}
}
