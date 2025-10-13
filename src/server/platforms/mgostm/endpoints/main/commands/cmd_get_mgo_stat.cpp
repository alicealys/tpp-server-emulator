#include <std_include.hpp>

#include "database/models/mgo_stats.hpp"

#include "cmd_get_mgo_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!data["target"].is_object())
		{
			return error(ERR_INVALIDARG);
		}

		auto has_id = false;
		auto target_player_opt = get_target_player(data["target"], &has_id);
		if (!has_id)
		{
			target_player_opt = player;
		}

		static const auto default_stats = std::make_shared<database::mgo_stats::stats_t>();

		if (target_player_opt.has_value())
		{
			const auto stats = database::mgo_stats::get_stats(target_player_opt->get_id());
			result["stat"] = stats->to_json();
		}
		else
		{
			result["stat"] = default_stats->to_json();
		}

		return result;
	}

	bool cmd_get_mgo_stat::needs_player()
	{
		return true;
	}
}
