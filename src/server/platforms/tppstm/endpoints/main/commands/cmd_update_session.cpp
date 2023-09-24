#include <std_include.hpp>

#include "cmd_update_session.hpp"

#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_update_session::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		if (!database::players::update_session(player.value()))
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		const auto active_sneak = database::players::find_active_sneak(player->get_id(), true, true);
		auto sneak_mode = active_sneak.has_value() ? 0 : -1;
		auto fob_index = active_sneak.has_value() ? 0 : -1;

		result["fob_index"] = sneak_mode;
		result["sneak_mode"] = fob_index;

		return result;
	}
}
