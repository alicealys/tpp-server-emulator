#include <std_include.hpp>

#include "cmd_get_fob_notice.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_notice::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.get())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto player_record = database::player_records::find(player->get_id());
		if (!player_data.get())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		result["active_event_server_text"] = "mb_fob_event_name_01";
		result["campaign_param_list"] = nlohmann::json::array();
		result["common_server_text"] = "NotImplement";
		result["common_server_text_title"] = "NotImplement";
		result["daily"] = 0;
		result["event_delete_date"] = 0;
		result["event_end_date"] = 0;
		result["exists_event_point_combat_deploy"] = 0;
		result["flag"] = 1497;

		result["league_update"]["get_point"] = 0;
		result["league_update"]["grade"] = 0;
		result["league_update"]["now_rank"] = 0;
		result["league_update"]["point"] = 0;
		result["league_update"]["prev_grade"] = 0;
		result["league_update"]["prev_rank"] = 0;
		result["league_update"]["score"] = 0;

		result["mb_coin"] = player_data->get_mb_coin();
		result["pf_current_season"] = 0;
		result["pf_finish_num"] = 0;
		result["pf_finish_num_max"] = 0;
		result["point_exchange_event_server_text"] = "NotImplement";

		result["record"]["defense"]["lose"] = player_record->get_defense_lose();
		result["record"]["defense"]["win"] = player_record->get_defense_lose();
		result["record"]["insurance"] = 0;
		result["record"]["score"] = 0;
		result["record"]["shield_date"] = player_record->get_shield_date();
		result["record"]["sneak"]["lose"] = player_record->get_sneak_lose();
		result["record"]["sneak"]["win"] = player_record->get_sneak_win();

		result["short_pf_current_season"] = 0;
		result["short_pf_finish_num"] = 0;
		result["short_pf_finish_num_max"] = 0;

		result["sneak_update"]["get_point"] = 0;
		result["sneak_update"]["now_rank"] = player_record->get_fob_rank();
		result["sneak_update"]["grade"] = player_record->get_fob_grade();
		result["sneak_update"]["prev_rank"] = player_record->get_prev_fob_rank();
		result["sneak_update"]["prev_grade"] = player_record->get_prev_fob_grade();
		result["sneak_update"]["point"] = 0;
		result["sneak_update"]["score"] = player_record->get_fob_point();
		
		database::player_records::sync_prev_values(player->get_id());

		return result;
	}
}
