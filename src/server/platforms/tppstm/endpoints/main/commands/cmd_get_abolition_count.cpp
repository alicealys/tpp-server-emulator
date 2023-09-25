#include <std_include.hpp>

#include "cmd_get_abolition_count.hpp"

#include "database/models/player_data.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_abolition_count::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";
		result["count"] = 3;
		result["date"] = std::time(nullptr);
		result["max"] = std::numeric_limits<int>::max();
		result["num"] = database::player_data::get_nuke_count();
		result["status"] = 0;

		return result;
	}
}
