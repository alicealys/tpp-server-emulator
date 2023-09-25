#include <std_include.hpp>

#include "cmd_active_sneak_mother_base.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_active_sneak_mother_base::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& mother_base_id_j = data["mother_base_id"];
		if (!mother_base_id_j.is_number_unsigned())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());

		if (!active_sneak.has_value() || active_sneak->get_fob_id() != mother_base_id)
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		if (!database::players::set_active_sneak(player->get_id(), active_sneak->get_fob_id(), active_sneak->get_owner_id(), 
			active_sneak->get_platform(), active_sneak->get_mode(), database::players::status_in_game_alert, active_sneak->is_sneak()))
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		return result;
	}
}
