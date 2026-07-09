#include <std_include.hpp>

#include "cmd_get_abolition_count.hpp"

#include "database/models/player_data.hpp"
#include "database/models/variables.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_abolition_count::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["info"]["count"] = database::variables::get<std::uint32_t>("abolition_count", 0u);
		result["info"]["date"] = database::variables::get<std::uint32_t>("abolition_date", 0u);
		result["info"]["max"] = std::numeric_limits<std::int32_t>::max();
		result["info"]["num"] = database::player_data::get_nuke_count();
		result["info"]["status"] = 0;

		return result;
	}
}
