#include <std_include.hpp>

#include "cmd_get_contribute_player_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_contribute_player_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		// contributors to nuclear abolition?
		result["info_list"] = nlohmann::json::array();
		result["info_num"] = 0;

		return result;
	}
}
