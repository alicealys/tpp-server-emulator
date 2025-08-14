#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_get_mgo_boost.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_boost::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());

		result["expire"] = "";
		result["boost_mag"] = mgo_data->get_xp_boost_mag();
		result["expire_unix_timestamp"] = mgo_data->get_xp_expire_unix_timestamp();

		return result;
	}

	bool cmd_get_mgo_boost::needs_player()
	{
		return true;
	}
}
