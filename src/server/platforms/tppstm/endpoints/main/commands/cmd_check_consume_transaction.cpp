#include <std_include.hpp>

#include "cmd_check_consume_transaction.hpp"

// unimplemeted

namespace tpp
{
	nlohmann::json cmd_check_consume_transaction::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
