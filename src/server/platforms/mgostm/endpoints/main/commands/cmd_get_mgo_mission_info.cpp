#include <std_include.hpp>

#include "cmd_get_mgo_mission_info.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_mission_info::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["gp_boost_mag"] = 0;
		result["xp_boost_mag"] = 0;

		result["rank_param"]["current_rank_xp"] = 0;
		result["rank_param"]["earned_rank_xp"] = 0;
		result["rank_param"]["rank_xp_list"] = nlohmann::json::array();

		for (auto i = 0; i < 16; i++)
		{
			result["rank_param"]["rank_xp_list"][i] = 0;
		}

		result["survival_params"]["current_survival_wins"] = 0;
		result["survival_params"]["earned_survival_gp"] = 0;
		result["survival_params"]["reward_category"] = "NotImplement";
		result["survival_params"]["reward_id_a"] = 0;
		result["survival_params"]["reward_id_b"] = 0;
		result["survival_params"]["reward_id_c"] = 0;
		result["survival_params"]["survival_update_key"] = 0;

		return result;
	}
}
