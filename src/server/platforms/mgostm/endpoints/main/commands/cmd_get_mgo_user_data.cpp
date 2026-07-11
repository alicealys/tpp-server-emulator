#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_get_mgo_user_data.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_user_data::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());

		result["gp"] = mgo_data->get_gp_coin();
		result["gp_boost_mag"] = mgo_data->get_gp_boost_mag();
		result["gp_expire"] = "NotImplement";
		result["gp_expire_unix_timestamp"] = mgo_data->get_gp_boost_expire().count();
		result["rank_xp"] = mgo_data->get_rank_xp();
		result["reward"]["reward_category"] = "";
		result["reward"]["reward_id_a"] = mgo_data->get_reward_id_a();
		result["reward"]["reward_id_b"] = mgo_data->get_reward_id_b();
		result["reward"]["reward_id_c"] = mgo_data->get_reward_id_c();
		result["survival_ticket_remain"] = mgo_data->get_survival_ticket_remain();
		result["xp_boost_mag"] = mgo_data->get_xp_boost_mag();
		result["xp_expire"] = "NotImplement";
		result["xp_expire_unix_timestamp"] = mgo_data->get_xp_boost_expire().count();

		return result;
	}

	bool cmd_get_mgo_user_data::needs_player()
	{
		return true;
	}
}
