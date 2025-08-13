#include <std_include.hpp>

#include "cmd_get_informationlist2.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
