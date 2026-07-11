#include <std_include.hpp>

#include "cmd_cancel_short_pfleague.hpp"

#include "database/models/pf_league.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_cancel_short_pfleague::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto current_application = database::pf_league::get_current_league_application(player->get_id());
		if (!current_application.has_value() || current_application->get_league_id() != 0)
		{
			return error(ERR_DATABASE);
		}

		if (!database::pf_league::remove_league_application(current_application->get_id()))
		{
			return error(ERR_DATABASE);
		}

		result["status"] = "SUCCESS";

		return result;
	}

	bool cmd_cancel_short_pfleague::needs_player()
	{
		return true;
	}
}
