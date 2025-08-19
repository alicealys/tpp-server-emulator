#include <std_include.hpp>

#include "cmd_get_fob_target_list.hpp"

#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"
#include "database/models/fobs.hpp"
#include "database/models/fob_event.hpp"

#include "cmd_get_fob_target_list/target_list_challenge.hpp"
#include "cmd_get_fob_target_list/target_list_deployed.hpp"
#include "cmd_get_fob_target_list/target_list_emergency.hpp"
#include "cmd_get_fob_target_list/target_list_enemy.hpp"
#include "cmd_get_fob_target_list/target_list_event.hpp"
#include "cmd_get_fob_target_list/target_list_follow.hpp"
#include "cmd_get_fob_target_list/target_list_follower.hpp"
#include "cmd_get_fob_target_list/target_list_fr_enemy.hpp"
#include "cmd_get_fob_target_list/target_list_nuclear.hpp"
#include "cmd_get_fob_target_list/target_list_pickup_high.hpp"
#include "cmd_get_fob_target_list/target_list_pickup.hpp"
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
		this->register_handler<target_list_nuclear>("NUCLEAR");
		this->register_handler<target_list_pickup_high>("PICKUP_HIGH");
		this->register_handler<target_list_pickup>("PICKUP");
		this->register_handler<target_list_trial>("TRIAL");
	}

	target_list_t cmd_get_fob_target_list::get_target_list(const std::string& name, const database::players::player& player, 
		const database::player_data::player_data_ptr& player_data, 
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
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}
		
		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.get())
		{
			return error(ERR_INVALIDARG);
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
		result["event_point"] = 0;

		const auto deploy_damage = player_data->get_fob_deploy_damage_param();
		if (deploy_damage.has_value())
		{
			result["fob_deploy_damage_param"] = deploy_damage.value();
		}
		else
		{
			result["fob_deploy_damage_param"]["cluster_index"] = 0;
			result["fob_deploy_damage_param"]["expiration_date"] = 0;
			result["fob_deploy_damage_param"]["motherbase_id"] = 0;

			for (auto i = 0; i < 16; i++)
			{
				result["fob_deploy_damage_param"]["damage_values"][i] = 0;
			}
		}

		result["win"] = stats->get_sneak_win();
		result["lose"] = stats->get_sneak_lose();

		result["shield_date"] = stats->get_shield_date();
		result["target_list"] = nlohmann::json::array();

		const auto target_list = get_target_list(type, player.value(), player_data, num);

		auto index = 0;
		for (const auto& target_entry : target_list)
		{
			const auto target_player = database::players::find(target_entry.player_id);
			const auto target_stats = database::player_records::find(target_entry.player_id);
			const auto target_fobs = database::fobs::get_fob_list(target_entry.player_id);
			const auto target_data = database::player_data::find(target_entry.player_id, true, false, true);

			if (!target_stats.has_value() || target_fobs.size() == 0 || !target_data.get())
			{
				continue;
			}

			auto target_mother_base = target_data->get_motherbase();
			auto& target = result["target_list"][index];

			target["attacker_emblem"]["parts"] = nlohmann::json::array();
			target["attacker_espionage"]["win"] = 0;
			target["attacker_espionage"]["lose"] = 0;
			target["attacker_espionage"]["score"] = 0;
			target["attacker_espionage"]["section"] = 0;

			target["attacker_info"] = player_info(0, 0);

			target["attacker_sneak_rank_grade"] = 0;
			target["cluster"] = 0;
			target["is_sneak_restriction"] = 0;
			target["is_win"] = 0;

			for (auto i = 0ull; i < target_fobs.size(); i++)
			{
				target["mother_base_param"][i]["area_id"] = 0;
				target["mother_base_param"][i]["cluster_param"] = nlohmann::json::array();
				target["mother_base_param"][i]["fob_index"] = 0;
				target["mother_base_param"][i]["price"] = 0;
				target["mother_base_param"][i]["construct_param"] = target_fobs[i].get_construct_param();
				target["mother_base_param"][i]["mother_base_id"] = target_fobs[i].get_id();
				target["mother_base_param"][i]["platform_count"] = target_fobs[i].get_platform_count();
				target["mother_base_param"][i]["security_rank"] = target_fobs[i].get_security_rank();
			}

			target["owner_detail_record"]["emblem"] = target_data->get_emblem();
			target["owner_detail_record"]["enemy"] = 0;
			target["owner_detail_record"]["espionage"]["win"] = target_stats->get_sneak_win();
			target["owner_detail_record"]["espionage"]["lose"] = target_stats->get_sneak_lose();
			target["owner_detail_record"]["espionage"]["score"] = target_stats->get_fob_point();
			target["owner_detail_record"]["espionage"]["section"] = 0;

			target["owner_detail_record"]["follow"] = 0;
			target["owner_detail_record"]["follower"] = 0;
			target["owner_detail_record"]["help"] = 0;
			target["owner_detail_record"]["hero"] = 0;
			target["owner_detail_record"]["insurance"] = 0;
			target["owner_detail_record"]["is_security_challenge"] = target_player->is_security_challenge_enabled();

			target["owner_detail_record"]["league_rank"]["grade"] = target_stats->get_league_grade();
			target["owner_detail_record"]["league_rank"]["rank"] = target_stats->get_league_rank();
			target["owner_detail_record"]["league_rank"]["score"] = target_stats->get_league_point();

			target["owner_detail_record"]["name_plate_id"] = target_mother_base["name_plate_id"];
			target["owner_detail_record"]["nuclear"] = target_data->get_nuke_count();
			target["owner_detail_record"]["online"] = 0;

			target["owner_detail_record"]["sneak_rank"]["grade"] = target_stats->get_fob_grade();
			target["owner_detail_record"]["sneak_rank"]["rank"] = target_stats->get_fob_rank();
			target["owner_detail_record"]["sneak_rank"]["score"] = target_stats->get_fob_point();
			
			target["owner_detail_record"]["staff_count"] = target_data->get_usable_staff_count();

			target["owner_fob_record"]["attack_count"] = 0;
			target["owner_fob_record"]["attack_gmp"] = 0;
			target["owner_fob_record"]["capture_nuclear"] = 0;
			target["owner_fob_record"]["capture_resource"]["biotic_resource"] = 0;
			target["owner_fob_record"]["capture_resource"]["common_metal"] = 0;
			target["owner_fob_record"]["capture_resource"]["fuel_resource"] = 0;
			target["owner_fob_record"]["capture_resource"]["minor_metal"] = 0;
			target["owner_fob_record"]["capture_resource"]["precious_metal"] = 0;

			target["owner_fob_record"]["capture_resource_count"] = 0;
			target["owner_fob_record"]["capture_staff"] = 0;
			target["owner_fob_record"]["capture_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			target["owner_fob_record"]["date_time"] = 0;
			target["owner_fob_record"]["injury_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			target["owner_fob_record"]["left_hour"] = 0;
			target["owner_fob_record"]["name_plate_id"] = target_mother_base["name_plate_id"];
			target["owner_fob_record"]["nuclear"] = target_data->get_nuke_count();

			const auto get_resource_value = [&](const std::uint32_t id)
			{
				const auto process_local = target_data->get_resource_value(database::player_data::processed_local, id);
				const auto process_server = target_data->get_resource_value(database::player_data::processed_server, id);
				const auto unprocess_local = target_data->get_resource_value(database::player_data::unprocessed_local, id);
				const auto unprocess_server = target_data->get_resource_value(database::player_data::unprocessed_server, id);
				return process_local + process_server + unprocess_local + unprocess_server;
			};

			target["owner_fob_record"]["processing_resource"]["biotic_resource"] = get_resource_value(0);
			target["owner_fob_record"]["processing_resource"]["common_metal"] = get_resource_value(1);
			target["owner_fob_record"]["processing_resource"]["fuel_resource"] = get_resource_value(2);
			target["owner_fob_record"]["processing_resource"]["minor_metal"] = get_resource_value(3);
			target["owner_fob_record"]["processing_resource"]["precious_metal"] = get_resource_value(4);

			auto& staff_counts = target["owner_fob_record"]["staff_count"];
			for (auto i = 0; i < 10; i++)
			{
				staff_counts[i] = 0;
			}

			for (auto i = 0u; i < target_data->get_staff_count(); i++)
			{
				const auto staff = target_data->get_staff(i);
				if (database::player_data::is_usable_staff(staff))
				{
					const auto value = staff_counts[staff.header.peak_rank].get<std::uint32_t>();
					staff_counts[staff.header.peak_rank] = value + 1;
				}
			}

			target["owner_fob_record"]["support_count"] = 0;
			target["owner_fob_record"]["supported_count"] = 0;
			target["owner_fob_record"]["usable_resource"]["biotic_resource"] = 0;
			target["owner_fob_record"]["usable_resource"]["common_metal"] = 0;
			target["owner_fob_record"]["usable_resource"]["fuel_resource"] = 0;
			target["owner_fob_record"]["usable_resource"]["minor_metal"] = 0;
			target["owner_fob_record"]["usable_resource"]["precious_metal"] = 0;

			target["owner_info"] = player_info(target_player);

			target["sneak_mode"] = 0;

			merge_json(target, target_entry.extra_data);

			++index;
		}

		result["target_num"] = index;
		result["type"] = type;

		return result;
	}
}
