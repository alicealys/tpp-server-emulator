#include <std_include.hpp>

#include "cmd_get_challenge_task_target_values.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_challenge_task_target_values::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";
		result["espionage_rating_grade"] = 0;
		result["fob_defense_success_count"] = 0;
		result["fob_deploy_to_supporters_emergency_count"] = 0;
		result["fob_sneak_count"] = 0;
		result["fob_sneak_success_count"] = 0;
		result["fob_supporting_user_count"] = 0;
		result["pf_rating_defense_force"] = 0;
		result["pf_rating_defense_life"] = 0;
		result["pf_rating_offence_force"] = 0;
		result["pf_rating_offence_life"] = 0;
		result["pf_rating_rank"] = 0;
		result["total_development_grade"] = 0;
		result["total_fob_security_level"] = 0;

		return result;
	}
}
