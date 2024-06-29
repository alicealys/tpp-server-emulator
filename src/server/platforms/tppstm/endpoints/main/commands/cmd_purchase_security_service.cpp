#include <std_include.hpp>

#include "cmd_purchase_security_service.hpp"

// unimplemented

namespace tpp
{
	nlohmann::json cmd_purchase_security_service::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "ERR_NOTIMPLEMENTED";
		return result;
	}
}
