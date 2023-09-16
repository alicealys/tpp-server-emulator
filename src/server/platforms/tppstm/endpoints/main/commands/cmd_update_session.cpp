#include <std_include.hpp>

#include "cmd_update_session.hpp"

#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_update_session::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["xuid"] = {};

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		if (!database::players::update_session(player->get_id()))
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		result["result"] = "NOERR";
		result["fob_index"] = -1;
		result["sneak_mode"] = -1;

		return result;
	}
}
