#include <std_include.hpp>

#include "cmd_get_troops_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_troops_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["progress"] = nlohmann::json::array();

		return result;
	}
}
