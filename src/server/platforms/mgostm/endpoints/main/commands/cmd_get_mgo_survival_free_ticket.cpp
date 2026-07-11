#include <std_include.hpp>

#include "cmd_get_mgo_survival_free_ticket.hpp"

#include "database/models/mgo_data.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_survival_free_ticket::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto mgo_data = database::mgo_data::find(player->get_id());
		if (!mgo_data.has_value())
		{
			result["remain"] = 0;
		}
		else
		{
			result["remain"] = mgo_data->get_survival_tickets();
		}

		return result;
	}

	bool cmd_get_mgo_survival_free_ticket::needs_player()
	{
		return true;
	}
}
