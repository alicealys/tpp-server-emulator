#include <std_include.hpp>

#include "cmd_get_fob_target_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_stats.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_target_list::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		const auto stats = database::player_stats::find(player->get_id());
		if (!stats.has_value())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto& type_j = data["type"];
		const auto& num_j = data["num"];

		if (!type_j.is_string() || !num_j.is_number_integer())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto type = type_j.get<std::string>();
		const auto num = num_j.get<std::uint32_t>();

		result["enable_security_challenge"] = 0;
		result["esp_point"] = stats->get_fob_point();
		result["event_point"] = 0;

		result["fob_deploy_damage_param"]["cluster_index"] = 0;
		result["fob_deploy_damage_param"]["expiration_date"] = 0;
		result["fob_deploy_damage_param"]["motherbase_id"] = 0;

		for (auto i = 0; i < 16; i++)
		{
			result["fob_deploy_damage_param"]["damage_values"][i] = 0;
		}

		result["win"] = stats->get_sneak_win();
		result["lose"] = stats->get_sneak_lose();

		result["shield_date"] = std::time(nullptr);

		result["target_list"] = nlohmann::json::array();
		result["target_num"] = 0;
		result["type"] = type;

		return result;
	}
}
