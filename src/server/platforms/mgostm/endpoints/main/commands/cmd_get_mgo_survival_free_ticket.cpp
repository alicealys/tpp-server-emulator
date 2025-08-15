#include <std_include.hpp>

#include "cmd_get_mgo_survival_free_ticket.hpp"

// unimplemented

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_survival_free_ticket::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
