#include <std_include.hpp>

#include "database/models/mgo_stats.hpp"

#include "cmd_set_mgo_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& stat = data["stat"];
		if (!stat.is_object() || !stat["stat_list"].is_array() || !stat["rule_stat_list"].is_array())
		{
			return error(ERR_INVALIDARG);
		}

		auto stats = std::make_shared<database::mgo_stats::stats_t>();
		if (!stats->parse(stat))
		{
			return error(ERR_INVALIDARG);
		}

		if (!database::mgo_stats::set_stats(player->get_id(), stats))
		{
			return error(ERR_DATABASE);
		}

		return result;
	}

	bool cmd_set_mgo_stat::needs_player()
	{
		return true;
	}
}
