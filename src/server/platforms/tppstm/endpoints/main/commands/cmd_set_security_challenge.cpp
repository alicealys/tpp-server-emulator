#include <std_include.hpp>

#include "cmd_set_security_challenge.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_set_security_challenge::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

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

		database::player_records::clear_shield_date(player->get_id());

		return result;
	}
}
