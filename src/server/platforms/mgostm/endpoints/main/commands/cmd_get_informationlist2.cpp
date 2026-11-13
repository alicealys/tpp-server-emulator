#include <std_include.hpp>

#include "database/models/players.hpp"

#include "cmd_get_informationlist2.hpp"

namespace emulator::mgo
{
	std::vector<cmd_get_informationlist2::message_t> cmd_get_informationlist2::messages;

	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		auto list = utils::resources::load_json(RESOURCE_MGO_INFORMATIONLIST2);
		cmd_get_informationlist2_base::parse_list(cmd_get_informationlist2::messages, list);
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return cmd_get_informationlist2_base::generate(cmd_get_informationlist2::messages, data, player);
	}
}
