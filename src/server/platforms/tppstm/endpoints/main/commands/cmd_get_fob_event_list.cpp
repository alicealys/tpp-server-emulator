#include <std_include.hpp>

#include "cmd_get_fob_event_list.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_event_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["event_list"] = nlohmann::json::array();
		result["event_num"] = 0;

		return result;
	}
}
