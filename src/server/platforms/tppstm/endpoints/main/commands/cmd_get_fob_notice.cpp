#include <std_include.hpp>

#include "cmd_get_fob_notice.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/pf_league.hpp"

namespace emulator::tpp
{
	namespace
	{
		enum notice_flags_t
		{
			notice_daily_reward = 1 << 0,
			notice_pf_points = 1 << 1,
			notice_league = 1 << 2,
			notice_fob = 1 << 3,
			notice_event = 1 << 4,
			notice_event_points = 1 << 5,
			notice_unk7 = 1 << 6,
			notice_unk8 = 1 << 7,
			notice_league_battles = 1 << 8,
			notice_league_bonus = 1 << 9,
			notice_mb_coins = 1 << 10,
		};

		bool has_unacked_league_results(const database::players::player& player, const std::chrono::seconds last_ack)
		{
			const auto league = database::pf_league::get_current_pf_league();
			if (!league.has_value())
			{
				return false;
			}

			const auto self = database::pf_league::get_player_competitor_instance(league->get_id(), player.get_id());
			if (self.has_value())
			{
				return false;
			}

			const auto battles = database::pf_league::get_player_battles(self->get_bracket_id(), self->get_player_id());
			for (const auto& battle : battles)
			{
				if (battle.get_winner_state() != database::pf_league::battle_winner_none && last_ack < battle.get_date())
				{
					return true;
				}
			}

			return false;
		}
	}

	nlohmann::json cmd_get_fob_notice::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_record = database::player_records::find(player->get_id());
		if (!player_record.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto fob_event = database::fob_events::get_current_event();

		auto flag = notice_unk7 | notice_unk8 | notice_mb_coins;

		if (player_record->get_event_point() > 1000)
		{
			flag |= notice_event_points;
		}

		if (player_record->get_pf_point_add() > 0)
		{
			flag |= notice_pf_points | notice_league;
		}

		if (player_record->get_league_grade() != player_record->get_prev_league_grade() ||
			player_record->get_league_rank() != player_record->get_prev_league_rank())
		{
			flag |= notice_league;
		}

		if (player_record->get_fob_grade() != player_record->get_prev_fob_grade() ||
			player_record->get_fob_rank() != player_record->get_prev_fob_rank())
		{
			flag |= notice_fob;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - player_record->get_daily_last_ack();
		if (diff > 24h)
		{
			flag |= notice_daily_reward;
		}

		if (fob_event.has_value())
		{
			flag |= notice_event;
		}

		if (has_unacked_league_results(player.value(), player_record->get_league_last_ack()))
		{
			flag |= notice_league_battles;
		}

		result["active_event_server_text"] = fob_event.has_value() ? fob_event->server_text : "NotImplement";
		result["campaign_param_list"] = nlohmann::json::array();
		result["common_server_text"] = "NotImplement";
		result["common_server_text_title"] = "NotImplement";
		result["daily"] = 0;
		result["event_delete_date"] = fob_event.has_value() ? fob_event->date_range.end.count() : 0ull;
		result["event_end_date"] = fob_event.has_value() ? fob_event->date_range.end.count() : 0ull;
		result["exists_event_point_combat_deploy"] = 0;
		result["flag"] = flag;

		result["league_update"]["get_point"] = player_record->get_pf_point_add();
		result["league_update"]["grade"] = player_record->get_league_grade();
		result["league_update"]["now_rank"] = player_record->get_league_rank();
		result["league_update"]["prev_grade"] = player_record->get_prev_league_grade();
		result["league_update"]["prev_rank"] = player_record->get_prev_league_rank();
		result["league_update"]["point"] = player_record->get_pf_point();
		result["league_update"]["score"] = player_record->get_league_point();

		result["mb_coin"] = database::player_data::get_mb_coins(player->get_id());
		result["pf_current_season"] = 0;
		result["pf_finish_num"] = 0;
		result["pf_finish_num_max"] = 0;
		result["point_exchange_event_server_text"] = "NotImplement";

		result["record"]["defense"]["lose"] = player_record->get_defense_lose();
		result["record"]["defense"]["win"] = player_record->get_defense_win();
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
