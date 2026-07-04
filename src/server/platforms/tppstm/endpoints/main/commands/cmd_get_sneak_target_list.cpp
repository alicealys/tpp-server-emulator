#include <std_include.hpp>

#include "cmd_get_sneak_target_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_sneak_target_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["target_list"] = nlohmann::json::array();
		result["target_num"] = 0;

		return result;
	}
}
