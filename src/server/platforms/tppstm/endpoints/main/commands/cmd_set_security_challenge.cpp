#include <std_include.hpp>

#include "cmd_set_security_challenge.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_set_security_challenge::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto player_record = database::player_records::find(player->get_id());
		if (!player_record.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto enabled = data["status"] == "ENABLE";
		if (enabled && player_record->get_is_insurance())
		{
			return error(ERR_IN_CONTRACT);
		}

		database::players::set_security_challenge(player->get_id(), enabled);
		database::player_records::clear_shield_date(player->get_id());

		return result;
	}

	bool cmd_set_security_challenge::needs_player()
	{
		return true;
	}
}
