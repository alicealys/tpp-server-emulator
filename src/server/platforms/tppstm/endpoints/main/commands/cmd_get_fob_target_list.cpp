#include <std_include.hpp>

#include "cmd_get_fob_target_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
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
		
		const auto stats = database::player_records::find(player->get_id());
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

		result["shield_date"] = stats->get_shield_date();

		const auto target_list = database::players::get_player_list(std::min(num, 30u));

		auto index = 0;
		for (const auto& target_player : target_list)
		{
			const auto target_stats = database::player_records::find(target_player.get_id());
			const auto target_fobs = database::fobs::get_fob_list(target_player.get_id());
			const auto target_data = database::player_data::find(target_player.get_id(), true, false, true);

			if (!target_stats.has_value() || target_fobs.size() == 0 || !target_data.get())
			{
				continue;
			}

			auto target_mother_base = target_data->get_motherbase();

			result["target_list"][index]["attacker_emblem"]["parts"] = nlohmann::json::array();
			result["target_list"][index]["attacker_espionage"]["win"] = 0;
			result["target_list"][index]["attacker_espionage"]["lose"] = 0;
			result["target_list"][index]["attacker_espionage"]["score"] = 0;
			result["target_list"][index]["attacker_espionage"]["section"] = 0;

			result["target_list"][index]["attacker_info"]["npid"]["handler"]["data"] = "";
			result["target_list"][index]["attacker_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
			result["target_list"][index]["attacker_info"]["npid"]["handler"]["term"] = 0;
			result["target_list"][index]["attacker_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			result["target_list"][index]["attacker_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
			result["target_list"][index]["attacker_info"]["player_id"] = 0;
			result["target_list"][index]["attacker_info"]["player_name"] = "NotImplement";

			result["target_list"][index]["attacker_sneak_rank_grade"] = 0;
			result["target_list"][index]["cluster"] = 0;
			result["target_list"][index]["is_sneak_restriction"] = 0;
			result["target_list"][index]["is_win"] = 0;

			for (auto i = 0; i < target_fobs.size(); i++)
			{
				result["target_list"][index]["mother_base_param"][i]["area_id"] = 0;
				result["target_list"][index]["mother_base_param"][i]["fob_index"] = 0;
				result["target_list"][index]["mother_base_param"][i]["price"] = 0;
				result["target_list"][index]["mother_base_param"][i]["construct_param"] = target_fobs[i].get_construct_param();
				result["target_list"][index]["mother_base_param"][i]["mother_base_id"] = target_fobs[i].get_id();
				result["target_list"][index]["mother_base_param"][i]["platform_count"] = target_fobs[i].get_platform_count();
				result["target_list"][index]["mother_base_param"][i]["security_rank"] = target_fobs[i].get_security_rank();
			}

			result["target_list"][index]["owner_detail_record"]["emblem"] = target_data->get_emblem();
			result["target_list"][index]["owner_detail_record"]["enemy"] = 0;
			result["target_list"][index]["owner_detail_record"]["espionage"]["win"] = target_stats->get_sneak_win();
			result["target_list"][index]["owner_detail_record"]["espionage"]["lose"] = target_stats->get_sneak_lose();
			result["target_list"][index]["owner_detail_record"]["espionage"]["score"] = target_stats->get_fob_point();
			result["target_list"][index]["owner_detail_record"]["espionage"]["section"] = 0;

			result["target_list"][index]["owner_detail_record"]["follow"] = 0;
			result["target_list"][index]["owner_detail_record"]["follower"] = 0;
			result["target_list"][index]["owner_detail_record"]["help"] = 0;
			result["target_list"][index]["owner_detail_record"]["hero"] = 0;
			result["target_list"][index]["owner_detail_record"]["insurance"] = 0;
			result["target_list"][index]["owner_detail_record"]["is_security_challenge"] = 0;

			result["target_list"][index]["owner_detail_record"]["league_rank"]["grade"] = 0;
			result["target_list"][index]["owner_detail_record"]["league_rank"]["rank"] = 0;
			result["target_list"][index]["owner_detail_record"]["league_rank"]["score"] = 0;

			result["target_list"][index]["owner_detail_record"]["name_plate_id"] = target_mother_base["name_plate_id"];
			result["target_list"][index]["owner_detail_record"]["nuclear"] = target_data->get_nuke_count();
			result["target_list"][index]["owner_detail_record"]["online"] = 0;

			result["target_list"][index]["owner_detail_record"]["sneak_rank"]["grade"] = 0;
			result["target_list"][index]["owner_detail_record"]["sneak_rank"]["rank"] = 0;
			result["target_list"][index]["owner_detail_record"]["sneak_rank"]["score"] = 0;
			
			result["target_list"][index]["owner_detail_record"]["staff_count"] = target_data->get_usable_staff_count();

			result["target_list"][index]["owner_fob_record"]["attack_count"] = 0;
			result["target_list"][index]["owner_fob_record"]["attack_gmp"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_nuclear"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_resource"]["biotic_resource"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_resource"]["common_metal"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_resource"]["fuel_resource"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_resource"]["minor_metal"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_resource"]["precious_metal"] = 0;

			result["target_list"][index]["owner_fob_record"]["capture_resource_count"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_staff"] = 0;
			result["target_list"][index]["owner_fob_record"]["capture_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			result["target_list"][index]["owner_fob_record"]["date_time"] = 0;
			result["target_list"][index]["owner_fob_record"]["injury_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			result["target_list"][index]["owner_fob_record"]["left_hour"] = 0;
			result["target_list"][index]["owner_fob_record"]["name_plate_id"] = target_mother_base["name_plate_id"];
			result["target_list"][index]["owner_fob_record"]["nuclear"] = target_data->get_nuke_count();

			result["target_list"][index]["owner_fob_record"]["processing_resource"]["biotic_resource"] = 0;
			result["target_list"][index]["owner_fob_record"]["processing_resource"]["common_metal"] = 0;
			result["target_list"][index]["owner_fob_record"]["processing_resource"]["fuel_resource"] = 0;
			result["target_list"][index]["owner_fob_record"]["processing_resource"]["minor_metal"] = 0;
			result["target_list"][index]["owner_fob_record"]["processing_resource"]["precious_metal"] = 0;

			auto& staff_counts = result["target_list"][index]["owner_fob_record"]["staff_count"];
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

			result["target_list"][index]["owner_fob_record"]["support_count"] = 0;
			result["target_list"][index]["owner_fob_record"]["supported_count"] = 0;
			result["target_list"][index]["owner_fob_record"]["usable_resource"]["biotic_resource"] = 0;
			result["target_list"][index]["owner_fob_record"]["usable_resource"]["common_metal"] = 0;
			result["target_list"][index]["owner_fob_record"]["usable_resource"]["fuel_resource"] = 0;
			result["target_list"][index]["owner_fob_record"]["usable_resource"]["minor_metal"] = 0;
			result["target_list"][index]["owner_fob_record"]["usable_resource"]["precious_metal"] = 0;

			result["target_list"][index]["owner_info"]["npid"]["handler"]["data"] = "";
			result["target_list"][index]["owner_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
			result["target_list"][index]["owner_info"]["npid"]["handler"]["term"] = 0;
			result["target_list"][index]["owner_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			result["target_list"][index]["owner_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
			result["target_list"][index]["owner_info"]["player_id"] = player->get_id();
			result["target_list"][index]["owner_info"]["player_name"] = std::format("{}_player01", target_player.get_account_id());
			result["target_list"][index]["owner_info"]["ugc"] = 1;
			result["target_list"][index]["owner_info"]["xuid"] = target_player.get_account_id();

			++index;
		}

		result["target_num"] = index;
		result["type"] = type;

		return result;
	}
}
