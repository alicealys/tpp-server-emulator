#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_get_mgo_gp.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_gp::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["gp"] = database::mgo_data::get_gp_coins(player->get_id());
		return result;
	}

	bool cmd_get_mgo_gp::needs_player()
	{
		return true;
	}
}
