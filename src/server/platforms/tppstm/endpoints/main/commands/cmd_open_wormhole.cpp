#include <std_include.hpp>

#include "cmd_open_wormhole.hpp"

#include "database/models/players.hpp"
#include "database/models/wormholes.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_open_wormhole::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& player_id_j = data["player_id"];
		const auto& to_player_id_j = data["to_player_id"];
		const auto& retaliate_score_j = data["retaliate_score"];
		const auto& flag_j = data["flag"];
		const auto& is_open_j = data["is_open"];

		if (!player_id_j.is_number_unsigned() || !to_player_id_j.is_number_unsigned() || 
			!retaliate_score_j.is_number_unsigned() || !flag_j.is_string() || !is_open_j.is_number_unsigned())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		if (player_id_j != player->get_id())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}
		
		const auto to_player_id = to_player_id_j.get<std::uint64_t>();
		const auto retaliate_score = to_player_id_j.get<std::uint32_t>();
		const auto flag = flag_j.get<std::string>();
		const auto flag_id = database::wormholes::get_flag_id(flag);

		if (flag_id == database::wormholes::wormhole_flag_invalid)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		database::wormholes::add_wormhole(player->get_id(), to_player_id, flag_id, retaliate_score);

		return result;
	}
}
