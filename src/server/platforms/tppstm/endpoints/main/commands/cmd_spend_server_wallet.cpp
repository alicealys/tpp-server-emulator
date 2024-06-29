#include <std_include.hpp>

#include "cmd_spend_server_wallet.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_spend_server_wallet::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
