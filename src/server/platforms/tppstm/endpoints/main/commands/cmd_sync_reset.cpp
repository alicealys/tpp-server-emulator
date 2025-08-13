#include <std_include.hpp>

#include "cmd_sync_reset.hpp"

// unimplemented

namespace emulator::tpp
{
	nlohmann::json cmd_sync_reset::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
