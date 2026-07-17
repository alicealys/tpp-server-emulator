#include <std_include.hpp>

#include "cmd_get_league_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_league_result::generate(nlohmann::json& data, 
		const std::optional<database::players::player>& player,
		const std::optional<database::pf_league::pf_league>& league)
	{
		nlohmann::json result;

		result["info"]["begin"] = 0;
		result["info"]["current"] = 0;
		result["info"]["day_battle"] = 0;
		result["info"]["end"] = 0;
		result["info"]["next"] = 0;
		result["info"]["history_count"] = 0;
		result["info"]["player_count"] = 0;
		result["info"]["point"] = 0;
		result["info"]["section"] = 0;
		result["info"]["match_history"] = nlohmann::json::array();
		result["info"]["player_info"] = nlohmann::json::array();

		if (!league.has_value())
		{
			return result;
		}

		const auto player_record = database::player_records::find(player->get_id());

		const auto self = database::pf_league::get_player_competitor_instance(league->get_id(), player->get_id());
		if (!player_record.has_value() || !self.has_value())
		{
			return result;
		}

		const auto pf_players = database::pf_league::get_players_in_bracket(self->get_bracket_id());
		auto idx = 0u;
		for (auto i = 0u; i < pf_players.size(); i++)
		{
			const auto& pf_competitor = pf_players[i];
			const auto competitor_id = pf_competitor.get_player_id();

			database::pf_league::player_pf_data_t competitor_params{};

			const auto competitor = database::players::find(competitor_id);
			const auto competitor_record = database::player_records::find(competitor_id);
			const auto competitor_data = database::player_data::find(competitor_id);
			competitor_params.fobs = database::fobs::get_fob_list(competitor_id);

			if (!competitor.has_value() || !competitor_record.has_value() || !competitor_data.has_value())
			{
				continue;
			}

			game::emblem_t emblem{};
			competitor_data->get_emblem(emblem);
			competitor_data->get_motherbase(competitor_params.motherbase);

			competitor_params.local_gmp = competitor_data->get_local_gmp();
			competitor_params.server_gmp = competitor_data->get_server_gmp();
			competitor_params.cumulative_grade = competitor_data->get_cumulative_grade();

			competitor_data->get_unit_levels(competitor_params.unit_levels);
			competitor_data->get_unit_counts(competitor_params.unit_counts);
			competitor_data->get_resource_arrays(competitor_params.resources);
			competitor_data->get_staff_array(competitor_params.staff);

			auto& entry = result["info"]["player_info"][idx++];

			database::pf_league::pf_battle fake_battle{};
			fake_battle.update_params(competitor_params, competitor_params);

			entry["attack_durability"] = fake_battle.get_attacker_durability();
			entry["attack_item"] = competitor_data->get_league_attack_item();
			entry["attack_level"] = competitor_params.unit_levels[game::unit_combat];
			entry["attack_lose"] = pf_competitor.get_attack_lose();
			entry["attack_nuclear"] = competitor_data->get_nuke_count();
			entry["attack_point"] = fake_battle.get_attacker_capability();
			entry["attack_staff"] = competitor_params.unit_counts[game::unit_combat];
			entry["attack_win"] = pf_competitor.get_attack_win();
			entry["conbat_point"] = 0;
			entry["cumulative_grade"] = competitor_data->get_cumulative_grade();
			entry["defence_durability"] = fake_battle.get_defender_durability();
			entry["defence_item"] = competitor_data->get_league_defense_item();
			entry["defence_level"] = competitor_params.unit_levels[game::unit_security];
			entry["defence_nuclear"] = competitor_data->get_nuke_count();
			entry["defence_point"] = fake_battle.get_defender_capability();
			entry["defence_staff"] = competitor_params.unit_counts[game::unit_security];
			entry["defense_lose"] = pf_competitor.get_defense_lose();
			entry["defense_win"] = pf_competitor.get_defense_win();
			entry["lose"] = pf_competitor.get_lose();

			auto security_rank = 0u;
			for (auto o = 0u; o < competitor_params.fobs.size(); o++)
			{
				entry["mother_base_param"][o]["area_id"] = 0;
				entry["mother_base_param"][o]["cluster_param"] = nlohmann::json::array();
				entry["mother_base_param"][o]["fob_index"] = 0;
				entry["mother_base_param"][o]["price"] = 0;
				entry["mother_base_param"][o]["construct_param"] = competitor_params.fobs[o].get_construct_param().packed;
				entry["mother_base_param"][o]["mother_base_id"] = competitor_params.fobs[o].get_id();
				entry["mother_base_param"][o]["platform_count"] = competitor_params.fobs[o].get_platform_count();
				entry["mother_base_param"][o]["security_rank"] = competitor_params.fobs[o].get_security_rank();

				const auto& cluster = competitor_params.fobs[o].get_cluster_param();
				for (auto l = 0u; l < game::fob_clusters_count; l++)
				{
					security_rank += cluster.param[l].cluster_security.fields.level;
				}
			}

			entry["narrow_lose"] = pf_competitor.get_narrow_lose();
			entry["narrow_win"] = pf_competitor.get_narrow_win();
			entry["planned_attack_item"] = 0;
			entry["planned_defence_item"] = 0;

			entry["player_detail_record"]["emblem"] = emblem.to_json();
			entry["player_detail_record"]["enemy"] = 0;
			entry["player_detail_record"]["espionage"]["lose"] = competitor_record->get_sneak_lose();
			entry["player_detail_record"]["espionage"]["score"] = competitor_record->get_fob_point();
			entry["player_detail_record"]["espionage"]["section"] = 0;
			entry["player_detail_record"]["espionage"]["win"] = competitor_record->get_sneak_win();
			entry["player_detail_record"]["follow"] = 0;
			entry["player_detail_record"]["follower"] = 0;
			entry["player_detail_record"]["help"] = 0;
			entry["player_detail_record"]["hero"] = 0;
			entry["player_detail_record"]["insurance"] = competitor_record->get_is_insurance();
			entry["player_detail_record"]["is_security_challenge"] = competitor->is_security_challenge_enabled();
			entry["player_detail_record"]["league_rank"]["grade"] = competitor_record->get_league_grade();
			entry["player_detail_record"]["league_rank"]["rank"] = competitor_record->get_league_rank();
			entry["player_detail_record"]["league_rank"]["score"] = competitor_record->get_league_point();
			entry["player_detail_record"]["name_plate_id"] = competitor_params.motherbase.name_plate_id;
			entry["player_detail_record"]["nuclear"] = competitor_data->get_nuke_count();
			entry["player_detail_record"]["online"] = 0;
			entry["player_detail_record"]["sneak_rank"]["grade"] = competitor_record->get_fob_grade();
			entry["player_detail_record"]["sneak_rank"]["rank"] = competitor_record->get_fob_rank();
			entry["player_detail_record"]["sneak_rank"]["score"] = competitor_record->get_fob_point();
			entry["player_detail_record"]["staff_count"] = competitor_data->get_staff_count();

			entry["player_info"] = player_info(competitor);
			entry["rank"] = i + 1;

			for (auto o = 0; o < 18; o++)
			{
				entry["result_history"][o] = 0;
			}

			entry["security_rank"] = security_rank;
			entry["win"] = pf_competitor.get_win();
			entry["winning_point"] = pf_competitor.get_victory_points();
		}

		auto battles = database::pf_league::get_player_battles(self->get_bracket_id(), self->get_player_id());
		for (auto i = 0u; i < battles.size(); i++)
		{
			auto& battle = battles[i];
			auto& entry = result["info"]["match_history"][i];

			const auto is_attacker = battle.get_attacker_id() == self->get_player_id();

			entry["attack_durability"] = battle.get_attacker_durability();
			entry["attack_item"] = battle.get_attacker_buff();
			entry["attack_level"] = battle.get_attacker_level();
			entry["attack_nuclear"] = battle.get_attacker_nuclear();
			entry["attack_pid"] = battle.get_attacker_id();
			entry["attack_point"] = battle.get_attacker_capability();
			entry["attack_staff"] = battle.get_attacker_staff();
			entry["cumulative_grade"] = battle.get_attacker_grade();
			entry["defence_durability"] = battle.get_defender_durability();
			entry["defence_item"] = battle.get_defender_buff();
			entry["defence_level"] = battle.get_defender_level();
			entry["defence_nuclear"] = battle.get_defender_nuclear();
			entry["defence_pid"] = battle.get_defender_id();
			entry["defence_point"] = battle.get_defender_capability();
			entry["defence_staff"] = battle.get_defender_staff();
			entry["match_date"] = battle.get_date().count();
			entry["result"] = battle.get_winner_state();
			entry["section"] = battle.get_section();
			entry["security_rank"] = battle.get_defender_security();
			entry["weather"] = 0;
			entry["win_point"] = is_attacker ? battle.get_attacker_points() : battle.get_defender_points();
		}

		result["info"]["player_count"] = idx;
		result["info"]["point"] = player_record->get_pf_point();
		result["info"]["section"] = league->get_id();

		return result;
	}

	nlohmann::json cmd_get_league_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto league = database::pf_league::get_current_pf_league();
		return cmd_get_league_result::generate(data, player, league);
	}

	bool cmd_get_league_result::needs_player()
	{
		return true;
	}
}
