#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/items.hpp"
#include "database/models/event_rankings.hpp"

#include "cmd_get_fob_event_point_exchange_params.hpp"
#include "cmd_get_pf_point_exchange_params.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_event_point_exchange_params::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return cmd_get_pf_point_exchange_params::generate(player, true);
	}
}
