#include <std_include.hpp>

#include "cmd_abort_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	void cmd_abort_mother_base::send_interrupted_sneak_result(const database::players::player& player)
	{
		const auto active_sneak = database::players::find_active_sneak_from_player(player.get_id());
		if (!active_sneak.has_value() || active_sneak->get_status() != database::players::status_in_game)
		{
			return;
		}

		const auto fob = database::fobs::get_fob(active_sneak->get_fob_id());
		if (!fob.has_value())
		{
			return;
		}

		database::sneak_results::sneak_result_data_t sneak_data{};
		sneak_data.cluster = static_cast<std::uint8_t>(active_sneak->get_cluster());

		database::sneak_results::add_sneak_result(player, fob.value(),
			active_sneak.value(), false, sneak_data, {});

		if (!active_sneak->is_security_challenge())
		{
			database::wormholes::add_wormhole(active_sneak->get_owner_id(), player.get_id(), 0u,
				database::wormholes::wormhole_flag_black, true, 30);
		}
	}

	void cmd_abort_mother_base::send_abort(const database::players::player& player)
	{
		cmd_abort_mother_base::send_interrupted_sneak_result(player);
		database::players::abort_mother_base(player.get_id());
	}

	nlohmann::json cmd_abort_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		cmd_abort_mother_base::send_abort(player.value());

		return result;
	}

	bool cmd_abort_mother_base::needs_player()
	{
		return true;
	}
}
