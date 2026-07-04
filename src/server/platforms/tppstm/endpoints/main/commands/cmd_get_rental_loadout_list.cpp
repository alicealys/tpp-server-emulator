#include <std_include.hpp>

#include "cmd_get_rental_loadout_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_rental_loadout_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["rental_list"] = nlohmann::json::array();
		result["list_num"] = 0;

		return result;
	}
}
