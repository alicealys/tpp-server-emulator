#include <std_include.hpp>

#include "cmd_use_mgo_survival_free_ticket.hpp"

#include "database/models/mgo_data.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_use_mgo_survival_free_ticket::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!database::mgo_data::spend_survival_tickets(player->get_id(), 1u))
		{
			return error(ERR_DATABASE);
		}

		result["remain"] = database::mgo_data::get_survival_tickets(player->get_id());

		return result;
	}

	bool cmd_use_mgo_survival_free_ticket::needs_player()
	{
		return true;
	}
}
