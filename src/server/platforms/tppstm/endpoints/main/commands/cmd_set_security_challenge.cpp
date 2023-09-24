#include <std_include.hpp>

#include "cmd_set_security_challenge.hpp"

#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_set_security_challenge::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto enabled = data["status"] == "ENABLE";
		if (!database::players::set_security_challenge(player->get_id(), enabled))
		{
			result["result"] = "ERR_DATABASE";
		}

		return result;
	}
}
