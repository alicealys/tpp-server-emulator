#include <std_include.hpp>

#include "cmd_get_mgo_title_usr2.hpp"
#include "cmd_get_mgo_title_usr.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_title_usr2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return cmd_get_mgo_title_usr::generate(data, player);
	}

	bool cmd_get_mgo_title_usr2::needs_player()
	{
		return true;
	}
}
