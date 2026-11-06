#include <std_include.hpp>

#include "target_list_deployed.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"

namespace emulator::tpp
{
	target_list_t target_list_deployed::generate(const database::players::player& player, const std::optional<database::player_data::player_data>& player_data, const std::uint32_t limit)
	{
		target_data_t target;

		const auto current_deploy = database::fob_deploy_assists::get_current_deploy(player.get_id());
		if (!current_deploy.has_value())
		{
			return {};
		}

		const auto fob = database::fobs::get_fob(current_deploy->get_fob_id());
		if (!fob.has_value())
		{
			return {};
		}

		target.player_id = fob->get_player_id();
		return {target};
	}
}
