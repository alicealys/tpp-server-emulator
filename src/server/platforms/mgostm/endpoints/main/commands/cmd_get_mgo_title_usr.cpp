#include <std_include.hpp>

#include "cmd_get_mgo_title_usr.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_title_usr::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& title_list = result["title_list"];

		title_list = utils::resources::load_json(RESOURCE_MGO_TITLE_LIST);
		for (auto i = 0ull; i < title_list.size(); i++)
		{
			title_list[i]["flag"] = 0;
		}

		return result;
	}
}
