#include <std_include.hpp>

#include "cmd_active_sneak_mother_base.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"
#include "database/models/fobs.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_active_sneak_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto& mother_base_id_j = data["mother_base_id"];
		if (!mother_base_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();

		const auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto owner = database::players::find(fob->get_player_id());
		if (!owner.has_value())
		{
			return error(ERR_DATABASE);
		}

		if (!owner->is_real_player())
		{
			return error(NOERR);
		}

		const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());

		if (!active_sneak.has_value() || active_sneak->get_fob_id() != mother_base_id)
		{
			return error(ERR_DATABASE);
		}

		if (!database::players::set_active_sneak(player->get_id(), active_sneak->get_fob_id(), active_sneak->get_owner_id(), 
			active_sneak->get_platform(), active_sneak->get_mode(), database::players::status_in_game, active_sneak->is_sneak(), false))
		{
			return error(ERR_DATABASE);
		}

		return error(NOERR);
	}
}
