#include <std_include.hpp>

#include "cmd_purchase_mgo_gear_color.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_purchase_mgo_gear_color::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
