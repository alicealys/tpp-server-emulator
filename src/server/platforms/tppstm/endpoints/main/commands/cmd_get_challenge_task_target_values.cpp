#include <std_include.hpp>

#include "cmd_get_challenge_task_target_values.hpp"

#include "database/models/player_records.hpp"
#include "database/models/player_follows.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_challenge_task_target_values::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto record = database::player_records::find(player->get_id());
		if (!record.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto follows = database::player_follows::get_follows(player->get_id());

		result["espionage_rating_grade"] = record->get_fob_point();
		result["fob_defense_success_count"] = record->get_defense_win();
		result["fob_deploy_to_supporters_emergency_count"] = 0;
		result["fob_sneak_count"] = record->get_sneak_win() + record->get_sneak_lose();
		result["fob_sneak_success_count"] = record->get_sneak_win();
		result["fob_supporting_user_count"] = follows.size();
		result["pf_rating_defense_force"] = 0;
		result["pf_rating_defense_life"] = 0;
		result["pf_rating_offence_force"] = 0;
		result["pf_rating_offence_life"] = 0;
		result["pf_rating_rank"] = record->get_league_grade();
		result["total_development_grade"] = 0;
		result["total_fob_security_level"] = 0;

		return result;
	}

	bool cmd_get_challenge_task_target_values::needs_player()
	{
		return true;
	}
}
