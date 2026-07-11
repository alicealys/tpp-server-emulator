#include <std_include.hpp>

#include "cmd_get_short_pfleague_result.hpp"
#include "cmd_get_league_result.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_short_pfleague_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto league = database::pf_league::get_current_short_pf_league();
		return cmd_get_league_result::generate(data, player, league);
	}

	bool cmd_get_short_pfleague_result::needs_player()
	{
		return true;
	}
}
