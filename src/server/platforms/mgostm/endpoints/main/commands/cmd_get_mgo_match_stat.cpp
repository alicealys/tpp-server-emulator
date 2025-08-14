#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_get_mgo_match_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_match_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());

		if (!mgo_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		result["played"] = mgo_data->get_matches_played();
		result["abandon"] = mgo_data->get_matches_abandoned();
		result["started"] = mgo_data->get_matches_started();

		return result;
	}

	bool cmd_get_mgo_match_stat::needs_player()
	{
		return true;
	}
}
