#include <std_include.hpp>

#include "database/models/players.hpp"

#include "cmd_get_informationlist2.hpp"

namespace emulator::mgo
{
	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		auto list = resource(RESOURCE_MGO_INFORMATIONLIST2);
		this->parse_list(list);
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto list = this->generate_list(data);
		result["info_list"] = list;
		result["info_num"] = list.size();

		return result;
	}
}
