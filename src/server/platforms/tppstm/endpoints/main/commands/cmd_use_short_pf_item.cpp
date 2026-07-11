#include <std_include.hpp>

#include "cmd_use_short_pf_item.hpp"
#include "cmd_use_pf_item.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_use_short_pf_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto league = database::pf_league::get_current_short_pf_league();
		return cmd_use_pf_item::generate(data, player, league);
	}

	bool cmd_use_short_pf_item::needs_player()
	{
		return true;
	}
}
