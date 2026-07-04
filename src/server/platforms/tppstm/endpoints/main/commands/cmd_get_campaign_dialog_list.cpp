#include <std_include.hpp>

#include "cmd_get_campaign_dialog_list.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_campaign_dialog_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		
		result["dialog_list"] = nlohmann::json::array();
		result["dialog_num"] = 0;

		return result;
	}
}
