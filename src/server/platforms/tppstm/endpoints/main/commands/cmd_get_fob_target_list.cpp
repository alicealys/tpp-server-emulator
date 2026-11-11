#include <std_include.hpp>

#include "cmd_get_fob_target_list.hpp"

#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"
#include "database/models/fobs.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/fob_deploy_assists.hpp"

#include "cmd_get_fob_target_list/target_list_challenge.hpp"
#include "cmd_get_fob_target_list/target_list_deployed.hpp"
#include "cmd_get_fob_target_list/target_list_emergency.hpp"
#include "cmd_get_fob_target_list/target_list_enemy.hpp"
#include "cmd_get_fob_target_list/target_list_event.hpp"
#include "cmd_get_fob_target_list/target_list_follow.hpp"
#include "cmd_get_fob_target_list/target_list_follower.hpp"
#include "cmd_get_fob_target_list/target_list_fr_enemy.hpp"
#include "cmd_get_fob_target_list/target_list_injury.hpp"
#include "cmd_get_fob_target_list/target_list_nuclear.hpp"
#include "cmd_get_fob_target_list/target_list_pickup.hpp"
#include "cmd_get_fob_target_list/target_list_pickup_high.hpp"
#include "cmd_get_fob_target_list/target_list_trial.hpp"

namespace emulator::tpp
{
	cmd_get_fob_target_list::cmd_get_fob_target_list()
	{
		this->register_handler<target_list_challenge>("CHALLENGE");
		this->register_handler<target_list_deployed>("DEPLOYED");
		this->register_handler<target_list_emergency>("EMERGENCY");
		this->register_handler<target_list_enemy>("ENEMY");
		this->register_handler<target_list_event>("EVENT");
		this->register_handler<target_list_follow>("FOLLOW");
		this->register_handler<target_list_follower>("FOLLOWER");
		this->register_handler<target_list_fr_enemy>("FR_ENEMY");
		this->register_handler<target_list_injury>("INJURY");
		this->register_handler<target_list_nuclear>("NUCLEAR");
		this->register_handler<target_list_pickup>("PICKUP");
		this->register_handler<target_list_pickup_high>("PICKUP_HIGH");
		this->register_handler<target_list_trial>("TRIAL");
	}

	target_list_t cmd_get_fob_target_list::get_target_list(const std::string& name, const database::players::player& player, 
		const std::optional<database::player_data::player_data>& player_data, 
		const std::uint32_t limit)
	{
		const auto iter = this->handlers_.find(name);
		if (iter == this->handlers_.end())
		{
			return {};
		}

		return iter->second->generate(player, player_data, limit);
	}

	nlohmann::json cmd_get_fob_target_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto& type_j = data["type"];
		const auto& num_j = data["num"];

		if (!type_j.is_string() || !num_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		const auto type = type_j.get<std::string>();
		const auto num = num_j.get<std::uint32_t>();

		result["enable_security_challenge"] = player->is_security_challenge_enabled();
		result["esp_point"] = stats->get_fob_point();
		result["event_point"] = stats->get_event_point();

		const auto current_deploy = database::fob_deploy_assists::get_current_deploy(player->get_id());
		if (current_deploy.has_value())
		{
			current_deploy->to_json(result["fob_deploy_damage_param"]);
		}
		else
		{
			database::fob_deploy_assists::deploy_params_t empty_params{};
			empty_params.to_json(result["fob_deploy_damage_param"]);
		}

		result["win"] = stats->get_sneak_win();
		result["lose"] = stats->get_sneak_lose();

		result["shield_date"] = stats->get_shield_date();
		result["target_list"] = nlohmann::json::array();

		auto target_list = get_target_list(type, player.value(), player_data, num);

		auto index = 0;
		for (auto& target_entry : target_list)
		{
			if (!target_entry.player.has_value())
			{
				target_entry.player = database::players::find(target_entry.player_id);
			}

			if (!target_entry.player_record.has_value())
			{
				target_entry.player_record = database::player_records::find(target_entry.player_id);
			}

			if (!target_entry.player_data.has_value())
			{
				target_entry.player_data = database::player_data::find(target_entry.player_id);
			}

			const auto target_fobs = database::fobs::get_fob_list(target_entry.player_id);

			if (!target_entry.player_record.has_value() || 
				target_fobs.size() == 0 || 
				!target_entry.player_data.has_value() ||
				!target_entry.player_record->has_an_fob())
			{
				continue;
			}

			auto& target = result["target_list"][index];

			target["attacker_emblem"]["parts"] = nlohmann::json::array();

			auto& attacker_espionage_j = target["attacker_espionage"];
			attacker_espionage_j["win"] = 0;
			attacker_espionage_j["lose"] = 0;
			attacker_espionage_j["score"] = 0;
			attacker_espionage_j["section"] = 0;

			target["attacker_info"] = player_info(0, 0);

			target["attacker_sneak_rank_grade"] = 0;
			target["cluster"] = 0;
			target["is_sneak_restriction"] = 0;
			target["is_win"] = 0;

			auto mother_base_index = 0;
			for (auto i = 0ull; i < target_fobs.size(); i++)
			{
				if (target_entry.fob_filter.has_value() && 
					!target_entry.fob_filter->operator()(target_fobs[i]))
				{
					continue;
				}

				auto idx = mother_base_index++;
				auto& param_j = target["mother_base_param"];
				param_j[idx]["area_id"] = 0;
				param_j[idx]["cluster_param"] = nlohmann::json::array();
				param_j[idx]["fob_index"] = 0;
				param_j[idx]["price"] = 0;
				param_j[idx]["construct_param"] = target_fobs[i].get_construct_param().packed;
				param_j[idx]["mother_base_id"] = target_fobs[i].get_id();
				param_j[idx]["platform_count"] = target_fobs[i].get_platform_count();
				param_j[idx]["security_rank"] = target_fobs[i].get_security_rank();
			}

			auto& detail_record_j = target["owner_detail_record"];

			target_entry.player_data->emblem.to_json(detail_record_j["emblem"]);

			auto& espionage_j = detail_record_j["espionage"];
			espionage_j["win"] = target_entry.player_record->get_sneak_win();
			espionage_j["lose"] = target_entry.player_record->get_sneak_lose();
			espionage_j["score"] = target_entry.player_record->get_fob_point();
			espionage_j["section"] = 0;

			detail_record_j["follow"] = 0;
			detail_record_j["follower"] = 0;
			detail_record_j["help"] = 0;
			detail_record_j["hero"] = 0;
			detail_record_j["enemy"] = 0;
			detail_record_j["insurance"] = 0;
			detail_record_j["is_security_challenge"] = target_entry.player->is_security_challenge_enabled();

			auto& league_rank_j = detail_record_j["league_rank"];
			league_rank_j["grade"] = target_entry.player_record->get_league_grade();
			league_rank_j["rank"] = target_entry.player_record->get_league_rank();
			league_rank_j["score"] = target_entry.player_record->get_league_point();

			detail_record_j["name_plate_id"] = target_entry.player_data->motherbase.name_plate_id;
			detail_record_j["nuclear"] = target_entry.player_data->get_nuke_count();
			detail_record_j["online"] = 0;

			auto& sneak_rank_j = detail_record_j["sneak_rank"];
			sneak_rank_j["grade"] = target_entry.player_record->get_fob_grade();
			sneak_rank_j["rank"] = target_entry.player_record->get_fob_rank();
			sneak_rank_j["score"] = target_entry.player_record->get_fob_point();
			
			detail_record_j["staff_count"] = target_entry.player_data->get_usable_staff_count();

			auto& owner_fob_record_j = target["owner_fob_record"];

			owner_fob_record_j["attack_count"] = 0;
			owner_fob_record_j["attack_gmp"] = 0;
			owner_fob_record_j["capture_nuclear"] = 0;

			auto& capture_resource_j = owner_fob_record_j["capture_resource"];
			capture_resource_j["biotic_resource"] = 0;
			capture_resource_j["common_metal"] = 0;
			capture_resource_j["fuel_resource"] = 0;
			capture_resource_j["minor_metal"] = 0;
			capture_resource_j["precious_metal"] = 0;

			owner_fob_record_j["capture_resource_count"] = 0;
			owner_fob_record_j["capture_staff"] = 0;
			owner_fob_record_j["capture_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			owner_fob_record_j["date_time"] = 0;
			owner_fob_record_j["injury_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			auto& staff_counts = target["owner_fob_record"]["staff_count"];
			for (auto i = 0; i < 10; i++)
			{
				staff_counts[i] = 0;
			}

			for (auto i = 0u; i < game::rank_count; i++)
			{
				staff_counts[i] = target_entry.player_data->get_staff_count_of_rank(i);
			}

			owner_fob_record_j["support_count"] = 0;
			owner_fob_record_j["supported_count"] = 0;

			owner_fob_record_j["left_hour"] = 0;
			owner_fob_record_j["name_plate_id"] = target_entry.player_data->motherbase.name_plate_id;
			owner_fob_record_j["nuclear"] = target_entry.player_data->get_nuke_count();

			database::player_data::resource_arrays_t target_resources{};
			target_entry.player_data->get_resource_arrays(target_resources);

			const auto get_processing_resource_value = [&](const std::uint32_t id)
			{
				const auto process_local = target_resources[game::unprocessed_local][id];
				const auto process_server = target_resources[game::unprocessed_server][id];
				return process_local + process_server;
			};

			const auto get_processed_resource_value = [&](const std::uint32_t id)
			{
				const auto process_local = target_resources[game::processed_local][id];
				const auto process_server = target_resources[game::processed_server][id];
				return process_local + process_server;
			};

			auto& processing_resource_j = owner_fob_record_j["processing_resource"];
			processing_resource_j["fuel_resource"] = get_processing_resource_value(game::FUEL_RESOURCE);
			processing_resource_j["biotic_resource"] = get_processing_resource_value(game::BIOTIC_RESOURCE);
			processing_resource_j["common_metal"] = get_processing_resource_value(game::COMMON_METAL);
			processing_resource_j["minor_metal"] = get_processing_resource_value(game::MINOR_METAL);
			processing_resource_j["precious_metal"] = get_processing_resource_value(game::PRECIOUS_METAL);

			auto& usable_resource_j = owner_fob_record_j["usable_resource"];
			usable_resource_j["biotic_resource"] = get_processed_resource_value(game::FUEL_RESOURCE);
			usable_resource_j["common_metal"] = get_processed_resource_value(game::BIOTIC_RESOURCE);
			usable_resource_j["fuel_resource"] = get_processed_resource_value(game::COMMON_METAL);
			usable_resource_j["minor_metal"] = get_processed_resource_value(game::MINOR_METAL);
			usable_resource_j["precious_metal"] = get_processed_resource_value(game::PRECIOUS_METAL);

			target["owner_info"] = player_info(target_entry.player);
			target["sneak_mode"] = 0;

			merge_json(target, target_entry.extra_data);

			++index;
		}

		result["target_num"] = index;
		result["type"] = type;

		return result;
	}

	bool cmd_get_fob_target_list::needs_player()
	{
		return true;
	}
}
