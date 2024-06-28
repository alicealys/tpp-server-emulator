#include <std_include.hpp>

#include "cmd_rental_loadout.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_rental_loadout::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
