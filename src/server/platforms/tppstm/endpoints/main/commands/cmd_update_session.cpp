#include <std_include.hpp>

#include "cmd_update_session.hpp"

#include "database/models/players.hpp"
#include "database/models/player_follows.hpp"
#include "database/models/fobs.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_update_session::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!database::players::update_session(player.value()))
		{
			return error(ERR_DATABASE);
		}

		result["fob_index"] = -1;
		result["sneak_mode"] = -1;

		const auto set_sneak = [&](const database::players::sneak_info& sneak, const bool is_support)
		{
			const auto fob = database::fobs::get_fob(sneak.get_fob_id());
			result["fob_index"] = fob->get_index();
			result["sneak_mode"] = sneak.get_game_sneak_mode();
		};

		const auto self_sneak = database::players::find_active_sneak(player->get_id(), true, true);
		if (self_sneak.has_value())
		{
			set_sneak(self_sneak.value(), false);
			return result;
		}

		const auto support_sneak = database::players::find_active_sneak_to_supporting(player->get_id(), true, true);
		if (support_sneak.has_value())
		{
			set_sneak(support_sneak.value(), false);
			return result;
		}

		return result;
	}

	bool cmd_update_session::needs_player()
	{
		return true;
	}
}
