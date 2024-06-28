#include <std_include.hpp>

#include "cmd_get_fob_target_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/wormholes.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	namespace
	{
#define CALLBACK_ARGS const database::players::player& player, const std::unique_ptr<database::player_data::player_data>& player_data, const std::uint32_t limit

		struct target_data_t
		{
			std::uint64_t player_id;
			nlohmann::json extra_data;
		};

		using target_list_t = std::vector<target_data_t>;

		target_list_t get_pickup_list(CALLBACK_ARGS)
		{
			const auto list = database::player_records::find_same_grade_players(player.get_id(), std::min(limit, 30u));
			target_list_t targets;

			for (const auto& row : list)
			{
				target_data_t target{};
				target.player_id = row.get_player_id();
				targets.emplace_back(target);
			}

			return targets;
		}

		target_list_t get_pickup_high_list(CALLBACK_ARGS)
		{
			const auto list = database::player_records::find_higher_grade_players(player.get_id(), std::min(limit, 30u));
			target_list_t targets;

			for (const auto& row : list)
			{
				target_data_t target{};
				target.player_id = row.get_player_id();
				targets.emplace_back(target);
			}

			return targets;
		}

		target_list_t get_enemy_list(CALLBACK_ARGS)
		{
			auto list = database::wormholes::find_active_wormholes(player.get_id());
			target_list_t targets;

			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			for (auto& [to_player_id, wormhole] : list)
			{
				target_data_t target{};

				auto left_hour = 0;
				if (wormhole.expire > now)
				{
					const auto diff = wormhole.expire - now;
					left_hour = std::chrono::duration_cast<std::chrono::hours>(diff).count();
				}

				target.extra_data["owner_detail_record"]["enemy"] = 1;
				target.extra_data["owner_fob_record"]["left_hour"] = left_hour;

				target.player_id = wormhole.to_player_id;

				targets.emplace_back(target);
			}

			return targets;
		}

		target_list_t get_injury_list(CALLBACK_ARGS)
		{
			auto list = database::sneak_results::get_sneak_results(player.get_id(), std::min(limit, 10u));
			target_list_t targets;

			const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch());

			for (auto& row : list)
			{
				target_data_t target{};

				auto& sneak_data = row.get_data();

				const auto wormhole = database::wormholes::get_wormhole_status(player.get_id(), row.get_player_id());

				if (wormhole.open)
				{
					auto left_hour = 0;
					if (wormhole.expire > now)
					{
						const auto diff = wormhole.expire - now;
						left_hour = std::chrono::duration_cast<std::chrono::hours>(diff).count();
					}

					target.extra_data["owner_detail_record"]["enemy"] = 1;
					target.extra_data["owner_fob_record"]["left_hour"] = left_hour;
				}

				target.extra_data["owner_fob_record"]["injury_staff_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				target.extra_data["is_win"] = static_cast<int>(row.is_win());
				target.extra_data["cluster"] = row.get_platform();

				for (auto i = 0; i < sneak_data["injure_soldier_id"].size(); i++)
				{
					const auto header_val = sneak_data["injure_soldier_id"][i]["param"][0].get<std::uint32_t>();
					database::player_data::staff_header_t header{};

					std::memcpy(&header, &header_val, sizeof(database::player_data::staff_header_t));
					auto& value = target.extra_data["owner_fob_record"]["injury_staff_count"][header.peak_rank];

					const auto current = value.get<std::uint32_t>();
					value = current + 1;
				}

				target.extra_data["owner_fob_record"]["date_time"] = row.get_date();
				target.player_id = row.get_player_id();

				targets.emplace_back(target);
			}

			return targets;
		}

		target_list_t get_challenge_list(CALLBACK_ARGS)
		{
			const auto list = database::players::find_with_security_challenge(std::min(limit, 10u));
			target_list_t targets;

			if (player.is_security_challenge_enabled())
			{
				target_data_t target{};
				target.player_id = player.get_id();
				targets.emplace_back(target);
			}

			for (auto& row : list)
			{
				if (row.get_id() != player.get_id())
				{
					target_data_t target{};
					target.player_id = row.get_id();
					targets.emplace_back(target);
				}
			}

			return targets;
		}

		target_list_t get_deployed_list(CALLBACK_ARGS)
		{
			target_data_t target;

			auto deploy_damage_opt = player_data->get_fob_deploy_damage_param();
			if (!deploy_damage_opt.has_value())
			{
				return {};
			}

			auto& deploy_damage = deploy_damage_opt.value();
			const auto& mother_base_id_j = deploy_damage["motherbase_id"];

			if (!mother_base_id_j.is_number_unsigned())
			{
				return {};
			}

			const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
			const auto fob = database::fobs::get_fob(mother_base_id);
			if (!fob.has_value())
			{
				return {};
			}

			target.player_id = fob->get_player_id();

			return {target};
		}

		target_list_t get_emergency_list(CALLBACK_ARGS)
		{
			const auto active_sneak = database::players::find_active_sneak(player.get_id(), true, true);
			if (!active_sneak.has_value())
			{
				return {};
			}

			target_list_t targets;
			target_data_t target;

			target.player_id = player.get_id();

			const auto attacker = database::players::find(active_sneak->get_player_id());
			const auto attacker_record = database::player_records::find(active_sneak->get_player_id());
			const auto attacker_data = database::player_data::find(active_sneak->get_player_id(), false, false, true);
			const auto fob_list = database::fobs::get_fob_list(active_sneak->get_owner_id());

			for (auto i = 0; i < fob_list.size(); i++)
			{
				auto& fob = fob_list[i];
				target.extra_data["mother_base_param"][i + 1]["area_id"] = 0;
				target.extra_data["mother_base_param"][i + 1]["construct_param"] = fob.get_construct_param();
				target.extra_data["mother_base_param"][i + 1]["fob_index"] = fob.get_index();
				target.extra_data["mother_base_param"][i + 1]["mother_base_id"] = fob.get_id();
				target.extra_data["mother_base_param"][i + 1]["platform_count"] = fob.get_platform_count();
				target.extra_data["mother_base_param"][i + 1]["price"] = 0;
				target.extra_data["mother_base_param"][i + 1]["security_rank"] = fob.get_security_rank();

				if (i == 0)
				{
					target.extra_data["mother_base_param"][0] = target.extra_data["mother_base_param"][1];
				}

				if (fob.get_id() == active_sneak->get_fob_id())
				{
					target.extra_data["mother_base_param"][0] = target.extra_data["mother_base_param"][i + 1];
				}
			}

			target.extra_data["attacker_emblem"] = attacker_data->get_emblem();
			target.extra_data["attacker_espionage"]["win"] = attacker_record->get_sneak_win();
			target.extra_data["attacker_espionage"]["lose"] = attacker_record->get_sneak_lose();
			target.extra_data["attacker_espionage"]["score"] = attacker_record->get_fob_point();
			target.extra_data["attacker_espionage"]["section"] = 0;

			target.extra_data["attacker_info"]["player_id"] = attacker->get_id();
			target.extra_data["attacker_info"]["player_name"] = std::format("{}_player01", attacker->get_account_id());
			target.extra_data["attacker_info"]["xuid"] = attacker->get_id();

			target.extra_data["attacker_sneak_rank_grade"] = attacker_record->get_fob_grade();

			targets.emplace_back(target);

			return targets;
		}

		using target_callback_t = std::function<target_list_t(CALLBACK_ARGS)>;
		std::unordered_map<std::string, target_callback_t> target_callbacks =
		{
			{"PICKUP", get_pickup_list},
			{"PICKUP_HIGH", get_pickup_high_list},
			{"ENEMY", get_enemy_list},
			{"INJURY", get_injury_list},
			{"CHALLENGE", get_challenge_list},
			{"DEPLOYED", get_deployed_list},
			{"EMERGENCY", get_emergency_list},
		};

		target_list_t get_target_list(const std::string& type, CALLBACK_ARGS)
		{
			const auto iter = target_callbacks.find(type);
			if (iter == target_callbacks.end())
			{
				return {};
			}

			return iter->second(player, player_data, limit);
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

			target["attacker_info"]["npid"]["handler"]["data"] = "";
			target["attacker_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
			target["attacker_info"]["npid"]["handler"]["term"] = 0;
			target["attacker_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["attacker_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["attacker_info"]["player_id"] = 0;
			target["attacker_info"]["player_name"] = "NotImplement";
			target["attacker_info"]["ugc"] = 1;
			target["attacker_info"]["xuid"] = 0;

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

			target["owner_info"]["npid"]["handler"]["data"] = "";
			target["owner_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
			target["owner_info"]["npid"]["handler"]["term"] = 0;
			target["owner_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["owner_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
			target["owner_info"]["player_id"] = target_player->get_id();
			target["owner_info"]["player_name"] = std::format("{}_player01", target_player->get_account_id());
			target["owner_info"]["ugc"] = 1;
			target["owner_info"]["xuid"] = target_player->get_account_id();

			target["sneak_mode"] = 0;

			merge_json(target, target_entry.extra_data);

			++index;
		}

		result["target_num"] = index;
		result["type"] = type;

		return result;
	}
}
