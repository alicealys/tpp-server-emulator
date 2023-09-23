#include <std_include.hpp>

#include "cmd_get_fob_target_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	namespace
	{
		std::vector<std::pair<std::uint64_t, nlohmann::json>> get_pickup_list(const database::players::player& player, const std::uint32_t limit)
		{
			const auto list = database::player_records::find_same_rank_players(player.get_id(), std::min(limit, 30u));
			std::vector<std::pair<std::uint64_t, nlohmann::json>> player_ids;

			for (const auto& row : list)
			{
				nlohmann::json data;
				player_ids.push_back(std::make_pair(row.get_player_id(), data));
			}

			return player_ids;
		}

		std::vector<std::pair<std::uint64_t, nlohmann::json>> get_pickup_high_list(const database::players::player& player, const std::uint32_t limit)
		{
			const auto list = database::player_records::find_higher_rank_players(player.get_id(), std::min(limit, 30u));
			std::vector<std::pair<std::uint64_t, nlohmann::json>> player_ids;

			for (const auto& row : list)
			{
				nlohmann::json data;
				player_ids.push_back(std::make_pair(row.get_player_id(), data));
			}

			return player_ids;
		}

		std::vector<std::pair<std::uint64_t, nlohmann::json>> get_injury_list(const database::players::player& player, const std::uint32_t limit)
		{
			auto list = database::sneak_results::get_sneak_results(player.get_id(), std::min(limit, 10u));
			std::vector<std::pair<std::uint64_t, nlohmann::json>> player_ids;

			for (auto& row : list)
			{
				nlohmann::json data;
				auto& sneak_data = row.get_data();

				data["owner_fob_record"]["injury_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				data["is_win"] = sneak_data["sneak_result"] == "WIN";

				for (auto i = 0; i < sneak_data["injure_soldier_id"].size(); i++)
				{
					const auto header_val = sneak_data["injure_soldier_id"][i]["param"][0].get<std::uint32_t>();
					database::player_data::staff_header_t header{};

					std::memcpy(&header, &header_val, sizeof(database::player_data::staff_header_t));
					auto& value = data["owner_fob_record"]["injury_staff_count"][header.peak_rank];

					const auto current = value.get<std::uint32_t>();
					value = current + 1;
				}

				data["owner_fob_record"]["date_time"] = row.get_date();

				player_ids.emplace_back(std::make_pair(row.get_player_id(), data));
			}

			return player_ids;
		}

		using target_callback_t = std::function<std::vector<std::pair<std::uint64_t, nlohmann::json>>(const database::players::player&, const std::uint32_t)>;
		std::unordered_map<std::string, target_callback_t> target_callbacks =
		{
			{"PICKUP", get_pickup_list},
			{"PICKUP_HIGH", get_pickup_high_list},
			{"INJURY", get_injury_list},
		};

		std::vector<std::pair<std::uint64_t, nlohmann::json>> get_target_list(const database::players::player& player,
			const std::string& type, const std::uint32_t limit)
		{
			const auto iter = target_callbacks.find(type);
			if (iter == target_callbacks.end())
			{
				return {};
			}

			return iter->second(player, limit);
		}

		void merge_json(nlohmann::json& data, const nlohmann::json& extra_data)
		{
			if (!extra_data.is_object())
			{
				return;
			}

			for (const auto& [k, v] : extra_data.items())
			{
				if (v.is_object())
				{
					if (data[k].is_object())
					{
						merge_json(data[k], v);
					}
					else
					{
						data[k] = v;
					}
				}
				else
				{
					data[k] = v;
				}
			}
		}
	}

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
		result["target_list"] = nlohmann::json::array();

		const auto target_list = get_target_list(player.value(), type, num);

		auto index = 0;
		for (const auto& [player_id, extra_data] : target_list)
		{
			const auto target_player = database::players::find(player_id);
			const auto target_stats = database::player_records::find(player_id);
			const auto target_fobs = database::fobs::get_fob_list(player_id);
			const auto target_data = database::player_data::find(player_id, true, false, true);

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

			target["attacker_info"]["npid"]["handler"]["data"] = "";
			target["attacker_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
			target["attacker_info"]["npid"]["handler"]["term"] = 0;
			target["attacker_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["attacker_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["attacker_info"]["player_id"] = 0;
			target["attacker_info"]["player_name"] = "NotImplement";

			target["attacker_sneak_rank_grade"] = 0;
			target["cluster"] = 0;
			target["is_sneak_restriction"] = 0;
			target["is_win"] = 0;

			for (auto i = 0; i < target_fobs.size(); i++)
			{
				target["mother_base_param"][i]["area_id"] = 0;
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
			target["owner_detail_record"]["is_security_challenge"] = 0;

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

			target["owner_info"]["npid"]["handler"]["data"] = "";
			target["owner_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
			target["owner_info"]["npid"]["handler"]["term"] = 0;
			target["owner_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["owner_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["owner_info"]["player_id"] = player->get_id();
			target["owner_info"]["player_name"] = std::format("{}_player01", target_player->get_account_id());
			target["owner_info"]["ugc"] = 1;
			target["owner_info"]["xuid"] = target_player->get_account_id();

			merge_json(target, extra_data);

			++index;
		}

		result["target_num"] = index;
		result["type"] = type;

		return result;
	}
}
