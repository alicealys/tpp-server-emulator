#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/fob_prisoners.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/event_rankings.hpp"
#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	namespace
	{
		enum soldier_array_action
		{
			soldier_kill = 0,
			soldier_injure = 1,
			soldier_capture = 2
		};

		struct soldier_t
		{
			std::uint64_t identifier;
			std::uint32_t count;
		};

		using soldier_map_t = std::unordered_map<std::uint64_t, std::uint32_t>;

		std::uint64_t make_soldier_identifier(const std::uint32_t header, const std::uint32_t seed)
		{
			std::uint32_t value[2]{};

			// ignore last 2 bits of header
			// ignore first 12 bits of seed

			value[0] |= header & 0x3FFFFFFF;
			value[1] |= seed & 0xFFFFF800;

			return *reinterpret_cast<std::uint64_t*>(&value);
		}

		std::uint64_t make_soldier_identifier(const game::staff_t& staff)
		{
			return make_soldier_identifier(staff.fields.packed_header, staff.fields.packed_seed);
		}

		soldier_map_t make_soldier_map(const std::vector<cmd_send_sneak_result::param_t::soldier_param_t>& list)
		{
			soldier_map_t map;

			for (const auto& entry : list)
			{
				soldier_t soldier{};
				soldier.identifier = make_soldier_identifier(entry.header, entry.seed);
				++map[soldier.identifier];
			}

			return map;
		}

		bool modify_staff_array(const std::uint64_t owner_id, 
			database::player_data::staff_array_container& staff_array, 
			std::vector<game::staff_t>& prisoners,
			soldier_map_t& soldier_ids, const std::size_t soldier_count, const soldier_array_action action, std::uint32_t* counts)
		{
			auto result = false;
			auto soldiers_left = soldier_count;

			for (auto i = 0u; i < game::max_staff_count; i++)
			{
				if (soldiers_left <= 0)
				{
					break;
				}

				auto staff = &staff_array[i];

				const auto identifier = make_soldier_identifier(*staff);
				auto iter = soldier_ids.find(identifier);
				if (iter == soldier_ids.end() || iter->second == 0)
				{
					continue;
				}

				--soldiers_left;
				--iter->second;

				if (!game::is_usable_staff(*staff))
				{
					continue;
				}

				switch (action)
				{
				case soldier_injure:
				{
					counts[staff->fields.header.peak_rank]++;
					staff->fields.status_sync.designation = game::SECTION_HOSPITAL;
					result = true;
					break;
				}
				case soldier_capture:
				{
					counts[staff->fields.header.peak_rank]++;
					prisoners.emplace_back(*staff);
					std::memset(staff, 0, sizeof(game::staff_t));
					result = true;
					break;
				}
				case soldier_kill:
				{
					counts[staff->fields.header.peak_rank]++;
					std::memset(staff, 0, sizeof(game::staff_t));
					result = true;
					break;
				}
				}
			}

			return result;
		}

		void check_disable_security_challenge(const database::player_data::player_data& player_data, 
			database::player_data::unit_counts_t& unit_counts)
		{
			const auto fob_list = database::fobs::get_fob_list(player_data.get_player_id());
			auto min_guards = 0u;

			for (const auto& fob : fob_list)
			{
				const auto& clusters = fob.get_cluster_param();
				for (auto i = 0u; i < game::fob_clusters_count; i++)
				{
					auto guard_count = 0u;
					guard_count += clusters.param[i].common_security[0].soldier;
					guard_count += clusters.param[i].common_security[1].soldier;
					guard_count += clusters.param[i].common_security[2].soldier;
					guard_count += clusters.param[i].unique_security.soldier;
					min_guards = std::max(min_guards, guard_count);
				}
			}

			if (unit_counts.value[game::unit_security] < min_guards)
			{
				database::players::set_security_challenge(player_data.get_player_id(), false);
			}
		}

		bool update_staff(const cmd_send_sneak_result::param_t& param, cmd_send_sneak_result::context_t& context)
		{
			database::player_data::staff_array_container owner_staff;
			context.owner_data->get_staff_array(owner_staff);

			auto modified_owner_staff = false;

			std::vector<game::staff_t> new_prisoners;

			if (!param.kill_soldier_id.empty())
			{
				auto kill_soldier_id = make_soldier_map(param.kill_soldier_id);
				modified_owner_staff |= modify_staff_array(context.owner_data->get_player_id(), owner_staff, new_prisoners,
					kill_soldier_id, param.kill_soldier_id.size(), soldier_kill, context.event_data.kill_staff_count);
			}

			if (!param.injure_soldier_id.empty())
			{
				auto injury_staff_id = make_soldier_map(param.injure_soldier_id);
				modified_owner_staff |= modify_staff_array(context.owner_data->get_player_id(), owner_staff, new_prisoners,
					injury_staff_id, param.injure_soldier_id.size(), soldier_injure, context.event_data.injury_staff_count);
			}

			if (!param.capture_soldier_id.empty())
			{
				auto capture_soldier_id = make_soldier_map(param.capture_soldier_id);
				modified_owner_staff |= modify_staff_array(context.owner_data->get_player_id(), owner_staff, new_prisoners,
					capture_soldier_id, param.capture_soldier_id.size(), soldier_capture, context.event_data.capture_staff_count);
			}

			database::fob_prisoners::add_prisoners(context.attacker_data->get_player_id(), context.owner->get_id(), new_prisoners);

			if (context.is_win)
			{
				database::fob_prisoners::direct_rescue(context.attacker_data->get_player_id());
				database::fob_prisoners::indirect_rescue(context.attacker_data->get_player_id());
			}

			if (!modified_owner_staff || context.owner_record->get_is_insurance())
			{
				return true;
			}

			database::player_data::staff_info_t staff_info{};
			if (!database::player_data::set_staff(context.owner->get_id(), context.owner_data->get_server_staff_version(),
				owner_staff, staff_info))
			{
				return false;
			}

			check_disable_security_challenge(context.owner_data.value(), staff_info.unit_counts);
			return true;
		}

		void update_resources(const cmd_send_sneak_result::param_t& param, cmd_send_sneak_result::context_t& context)
		{
			database::player_data::resource_arrays_t owner_resources{};
			database::player_data::resource_arrays_t attacker_resources{};

			context.owner_data->get_resource_arrays(owner_resources);
			context.attacker_data->get_resource_arrays(attacker_resources);

			const auto do_resource = [&](const std::uint32_t value, 
				const std::uint32_t resource_type, 
				const std::uint32_t resource_id, const std::uint32_t cap, bool destroy = false)
			{
				const auto max_count = game::resource_caps[resource_type][resource_id];
				const auto attacker_count = attacker_resources[resource_type][resource_id];

				const auto base_value = std::min(cap, value);
				const auto sub_value = std::min(owner_resources[resource_type][resource_id], base_value);
				auto add_value = 0u;
				if (attacker_count < max_count)
				{
					add_value = std::min(sub_value, max_count - attacker_count);
				}
				
				if (!context.owner_record->get_is_insurance() || resource_id == game::NUCLEAR_WEAPON)
				{
					owner_resources[resource_type][resource_id] -= sub_value;
				}

				if (!destroy)
				{
					attacker_resources[resource_type][resource_id] += add_value;
				}

				return sub_value;
			};

			constexpr const auto placement_cap = 8u * 3u + 10u;

			context.event_data.capture_nuclear = static_cast<std::uint8_t>(do_resource(param.capture_nuclear, game::processed_server, game::NUCLEAR_WEAPON, 4));
			
			context.event_data.capture_resource.biotic_resource = do_resource(param.capture_resource.biotic_resource, game::unprocessed_server, game::BIOTIC_RESOURCE, 50000u);
			context.event_data.capture_resource.common_metal = do_resource(param.capture_resource.common_metal, game::unprocessed_server, game::COMMON_METAL, 40000u);
			context.event_data.capture_resource.fuel_resource = do_resource(param.capture_resource.fuel_resource, game::unprocessed_server, game::FUEL_RESOURCE, 40000u);
			context.event_data.capture_resource.minor_metal = do_resource(param.capture_resource.minor_metal, game::unprocessed_server, game::MINOR_METAL, 25000u);
			context.event_data.capture_resource.precious_metal = do_resource(param.capture_resource.precious_metal, game::unprocessed_server, game::PRECIOUS_METAL, 4000u);

			context.event_data.capture_placement.mortar_normal = do_resource(param.capture_placement.mortar_normal, game::processed_server, game::MORTAR_NORMAL, placement_cap);
			context.event_data.capture_placement.gatling_gun_east = do_resource(param.capture_placement.gatling_gun_east, game::processed_server, game::ANTI_AIR_GATLING_GUN_EAST, placement_cap);
			context.event_data.capture_placement.gatling_gun_west = do_resource(param.capture_placement.gatling_gun_west, game::processed_server, game::ANTI_AIR_GATLING_GUN_WEST, placement_cap);
			context.event_data.capture_placement.emplacement_gun_east = do_resource(param.capture_placement.emplacement_gun_east, game::processed_server, game::EMPLACEMENT_GUN_EAST, placement_cap);
			context.event_data.capture_placement.emplacement_gun_west = do_resource(param.capture_placement.emplacement_gun_west, game::processed_server, game::EMPLACEMENT_GUN_WEST, placement_cap);

			context.event_data.destroy_placement.mortar_normal = do_resource(param.destroy_placement.mortar_normal, game::processed_server, game::MORTAR_NORMAL, placement_cap, true);
			context.event_data.destroy_placement.gatling_gun_east = do_resource(param.destroy_placement.gatling_gun_east, game::processed_server, game::ANTI_AIR_GATLING_GUN_EAST, placement_cap, true);
			context.event_data.destroy_placement.gatling_gun_west = do_resource(param.destroy_placement.gatling_gun_west, game::processed_server, game::ANTI_AIR_GATLING_GUN_WEST, placement_cap, true);
			context.event_data.destroy_placement.emplacement_gun_east = do_resource(param.destroy_placement.emplacement_gun_east, game::processed_server, game::EMPLACEMENT_GUN_EAST, placement_cap, true);
			context.event_data.destroy_placement.emplacement_gun_west = do_resource(param.destroy_placement.emplacement_gun_west, game::processed_server, game::EMPLACEMENT_GUN_WEST, placement_cap, true);
		
			database::player_data::set_resources(context.attacker_data->get_player_id(), attacker_resources);
			database::player_data::set_resources(context.owner_data->get_player_id(), owner_resources);
		}

		bool parse_event_log(nlohmann::json& event, std::string& parsed)
		{
			if (!event.is_object())
			{
				return false;
			}

			auto& data_j = event["data"];
			auto& size_j = event["size"];

			if (!data_j.is_string() || !size_j.is_number_unsigned())
			{
				return false;
			}

			const auto event_log = event["data"].get<std::string>();
			auto event_log_size = event["size"].get<std::size_t>();

			if (event_log_size > 0x4000 || event_log_size > event_log.size())
			{
				return false;
			}

			parsed = utils::cryptography::base64::decode(event_log);
			parsed.resize(event_log_size);
			return true;
		}

		bool parse_event_data(nlohmann::json& event, database::sneak_results::sneak_result_data_t& event_data)
		{
			const auto set_value = []<typename T>(nlohmann::json& v, T& val, const bool is_signed)
			{
				if ((is_signed && v.is_number_integer()) || (!is_signed && v.is_number_unsigned()))
				{
					val = v.get<T>();
				}
			};

			set_value(event["gmp"], event_data.gmp, true);
			set_value(event["layout_code"], event_data.layout_code, false);
			set_value(event["position_x"], event_data.position_x, true);
			set_value(event["position_z"], event_data.position_z, true);
			set_value(event["rotate_y"], event_data.rotate_y, true);
			set_value(event["cluster"], event_data.cluster, false);

			return true;
		}
	}

	void cmd_send_sneak_result::handle_rewards(const database::players::player& player,
		const param_t& param, cmd_send_sneak_result::context_t& context, nlohmann::json& rewards_j)
	{
		database::sneak_results::sneak_result_rewards_t reward_info{};

		if (!context.active_sneak->is_sneak() || context.active_sneak->get_mode() != database::players::mode_actual || 
			!context.is_win || context.active_sneak->get_owner_id() == player.get_id())
		{
			reward_info.to_json(rewards_j);
			return;
		}

		const auto platform = context.active_sneak->get_platform();
		const auto& clusters = context.fob->get_cluster_param();
		const auto cluster_type = game::cluster_index_map[platform];
		const auto cluster = clusters.param[cluster_type];

		std::vector<game::reward_t> rewards;
		std::vector<game::reward_t> result_rewards;
		database::sneak_results::generate_sneak_rewards(cluster, cluster_type, reward_info, rewards);

		context.attacker_data->give_rewards(rewards, &result_rewards);
		reward_info.set_rewards(result_rewards);
		reward_info.to_json(rewards_j);
	}

	bool cmd_send_sneak_result::handle_result(const database::players::player& player,
		const param_t& param, cmd_send_sneak_result::context_t& context)
	{
		if (context.active_sneak->get_mode() != database::players::mode_actual || 
			context.active_sneak->get_owner_id() == player.get_id())
		{
			return true;
		}

		if (context.active_sneak->is_sneak()) // attack
		{
			if (!database::vars.pvp_mode && !database::vars.no_fob_damage)
			{
				if (!update_staff(param, context))
				{
					return false;
				}

				update_resources(param, context);

				const auto materials_stolen = 
					context.event_data.capture_resource.biotic_resource +
					context.event_data.capture_resource.fuel_resource +
					context.event_data.capture_resource.common_metal +
					context.event_data.capture_resource.minor_metal +
					context.event_data.capture_resource.precious_metal;

				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::materials_stolen, materials_stolen);

				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::damage_defender, param.damage_point);
			}

			auto& active_sneak_val = context.active_sneak.value();
			if (!database::sneak_results::add_sneak_result(player, context.fob.value(),
				active_sneak_val, context.is_win, context.event_data, context.event_log))
			{
				return false;
			}

			if (!context.active_sneak->is_security_challenge())
			{
				database::player_records::set_shield_date(context.active_sneak->get_owner_id(), context.is_win);
			}

			if (context.is_win)
			{
				if (param.is_goal && param.is_perfect_stealth)
				{
					database::event_rankings::increment_event_value(player.get_id(), database::event_rankings::total_stealth, 1);
				}

				const auto defense_sneak = database::players::find_active_sneak(context.active_sneak->get_owner_id(), false, true);
				const auto event_id = defense_sneak.has_value()
					? database::event_rankings::cores_reached_defender
					: database::event_rankings::cores_reached_no_defender;
				database::event_rankings::increment_event_value(player.get_id(), event_id, 1);
			}
		}
		else // defense
		{
			if (param.count_of_neutralized_by_intruder > 0)
			{
				const auto c = std::min(100u, param.count_of_neutralized_by_intruder);
				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::times_neutralized_by_intruder, c);
			}

			if (param.count_of_neutralize_intruder > 0 && player.get_id() == context.active_sneak->get_owner_id())
			{
				const auto c = std::min(100u, param.count_of_neutralize_intruder);
				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::defender_neutralized_intruder, c);
			}

			if (context.is_win)
			{
				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::successful_defenses, 1);
			}

			if (context.active_sneak->get_owner_id() == player.get_id())
			{
				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::defense_deployments, 1);
			}
			else
			{
				database::event_rankings::increment_event_value(player.get_id(),
					database::event_rankings::defense_deployments_support, 1);

				database::event_rankings::increment_event_value(context.active_sneak->get_owner_id(),
					database::event_rankings::defense_deployments_support_received, 1);
			}
		}

		database::player_records::add_sneak_result(player.get_id(), context.fob->get_player_id(),
			param.sneak_point, context.is_win, context.active_sneak->is_sneak());

		if (param.sneak_point > 0)
		{
			database::event_rankings::set_value_if_bigger(player.get_id(),
				database::event_rankings::most_ep_in_mission, param.sneak_point);
		}

		return true;
	}

	void cmd_send_sneak_result::handle_result_event(const database::players::player& player, const param_t& param, context_t& context)
	{
		if (!param.is_event || context.active_sneak->get_mode() != database::players::mode_actual)
		{
			return;
		}

		database::player_records::add_event_points(player.get_id(), param.event_point);
		database::event_rankings::set_value_if_bigger(player.get_id(), database::event_rankings::most_ep_in_mission, param.sneak_point);
		database::event_rankings::increment_event_value(player.get_id(), database::event_rankings::fob_event_ranking, param.event_point);
		database::event_rankings::increment_event_value(player.get_id(), database::event_rankings::cores_reached_no_defender, 1);
		database::player_records::add_sneak_result(player.get_id(), context.fob->get_player_id(), param.sneak_point, context.is_win, true);
	}

	bool cmd_send_sneak_result::open_wormhole(const database::players::player& player, const param_t& param, context_t& context)
	{
		auto has_captured_staff = false;
		for (auto i = 0ull; i < ARRAYSIZE(context.event_data.capture_staff_count); i++)
		{
			if (context.event_data.capture_staff_count[i] > 0)
			{
				has_captured_staff = true;
			}
		}

		// observed behavior from konami server:
		// 
		// cmd_open_wormhole:	  "contributes" to the current wormhole by retaliate_point and is_open
		//						  current wormhole duration = sum(retaliate_point) in HOURS, is_open = max(is_open) of all contributions
		//						  left_hour on each side is equal to how much damage the opponent has done
		// 
		// cmd_send_sneak_result: if [result == WIN OR capture_staff_count > 0] => clear all of the attackers wormholes to the victim
		// 
		//						  if
		//							| capture_staff_count == 0 => opens wormhole with retaliate point 300 
		//							| capture_staff_count  > 0 => opens wormhole with retaliate point 720
		// 
		//						  if open_wormole == 1 => wormhole is flagged as open, it is created regardless
		// 

		auto revenge_point = database::wormholes::mid_retaliate_score;
		if (context.is_win || has_captured_staff)
		{
			database::wormholes::remove_wormholes(player.get_id(), context.owner->get_id());
			revenge_point = database::wormholes::max_retaliate_score;
		}

		database::wormholes::add_wormhole(context.owner->get_id(), player.get_id(), 0ull,
			database::wormholes::wormhole_flag_black, param.open_wormhole, revenge_point);

		const auto status = database::wormholes::get_wormhole_status(context.owner->get_id(), player.get_id());
		return status.is_open();
	}

	bool cmd_send_sneak_result::param_t::parse(nlohmann::json& data)
	{
		auto& is_event_j = data["is_event"];
		auto& is_goal_j = data["is_goal"];
		auto& is_perfect_stealth_j = data["is_perfect_stealth"];
		auto& is_plus_j = data["is_plus"];
		auto& is_sneak_j = data["is_sneak"];
		auto& is_supporter_j = data["is_supporter"];
		auto& open_wormhole_j = data["open_wormhole"];
		auto& retaliate_wormhole_j = data["retaliate_wormhole"];
		auto& result_type_j = data["result_type"];

		if (!is_event_j.is_number_unsigned() ||
			!is_goal_j.is_number_unsigned() ||
			!is_perfect_stealth_j.is_number_unsigned() ||
			!is_plus_j.is_number_unsigned() ||
			!is_sneak_j.is_number_unsigned() ||
			!is_supporter_j.is_number_unsigned() ||
			!open_wormhole_j.is_number_unsigned() ||
			!retaliate_wormhole_j.is_number_unsigned() ||
			!result_type_j.is_number_unsigned())
		{
			return false;
		}

		auto& cleared_plant_count_j = data["cleared_plant_count"];
		auto& count_of_neutralize_intruder_j = data["count_of_neutralize_intruder"];
		auto& count_of_neutralized_by_intruder_j = data["count_of_neutralized_by_intruder"];
		auto& damage_point_j = data["damage_point"];
		auto& retaliate_point_j = data["retaliate_point"];
		auto& event_version_j = data["event_version"];
		auto& high_rank_j = data["high_rank"];
		auto& mission_task_complete_bits_j = data["mission_task_complete_bits"];

		if (!cleared_plant_count_j.is_number_unsigned() ||
			!count_of_neutralize_intruder_j.is_number_unsigned() ||
			!count_of_neutralized_by_intruder_j.is_number_unsigned() ||
			!damage_point_j.is_number_unsigned() ||
			!retaliate_point_j.is_number_unsigned() ||
			!event_version_j.is_number_unsigned() ||
			!high_rank_j.is_number_unsigned() ||
			!mission_task_complete_bits_j.is_number_unsigned())
		{
			return false;
		}

		auto& event_point_j = data["event_point"];
		auto& sneak_point_j = data["sneak_point"];
		auto& mother_base_id_j = data["mother_base_id"];
		auto& mode_j = data["mode"];
		auto& sneak_result_j = data["sneak_result"];

		if (!event_point_j.is_number_integer() ||
			!sneak_point_j.is_number_integer() ||
			!mother_base_id_j.is_number_unsigned() ||
			!mode_j.is_string() ||
			!sneak_result_j.is_string())
		{
			return false;
		}

		auto& capture_placement_j = data["capture_placement"];
		auto& destroy_placement_j = data["destroy_placement"];
		auto& capture_resource_j = data["capture_resource"];
		auto& recover_resource_j = data["recover_resource"];
		auto& capture_nuclear_j = data["capture_nuclear"];

		if (!this->capture_placement.parse(capture_placement_j) ||
			!this->destroy_placement.parse(destroy_placement_j) ||
			!this->capture_resource.parse(capture_resource_j) ||
			!this->recover_resource.parse(recover_resource_j) || 
			!capture_nuclear_j.is_number_unsigned())
		{
			return false;
		}

		auto& injure_soldier_id_j = data["injure_soldier_id"];
		auto& capture_soldier_id_j = data["capture_soldier_id"];
		auto& kill_soldier_id_j = data["kill_soldier_id"];

		this->capture_nuclear = capture_nuclear_j.get<std::uint32_t>();

		this->is_event = is_event_j.get<std::uint8_t>();
		this->is_goal = is_goal_j.get<std::uint8_t>();
		this->is_perfect_stealth = is_perfect_stealth_j.get<std::uint8_t>();
		this->is_plus = is_plus_j.get<std::uint8_t>();
		this->is_sneak = is_sneak_j.get<std::uint8_t>();
		this->is_supporter = is_supporter_j.get<std::uint8_t>();
		this->open_wormhole = open_wormhole_j.get<std::uint8_t>();
		this->retaliate_wormhole = retaliate_wormhole_j.get<std::uint8_t>();
		this->result_type = result_type_j.get<std::uint8_t>();

		this->cleared_plant_count = cleared_plant_count_j.get<std::uint32_t>();
		this->count_of_neutralize_intruder = count_of_neutralize_intruder_j.get<std::uint32_t>();
		this->count_of_neutralized_by_intruder = count_of_neutralized_by_intruder_j.get<std::uint32_t>();
		this->damage_point = damage_point_j.get<std::uint32_t>();
		this->retaliate_point = retaliate_point_j.get<std::uint32_t>();
		this->event_version = event_version_j.get<std::uint32_t>();
		this->high_rank = high_rank_j.get<std::uint32_t>();
		this->mission_task_complete_bits = mission_task_complete_bits_j.get<std::uint32_t>();

		this->sneak_point = sneak_point_j.get<std::int32_t>();
		this->event_point = event_point_j.get<std::int32_t>();

		this->mother_base_id = mother_base_id_j.get<std::uint64_t>();

		this->sneak_result = sneak_result_j.get<std::string>();
		this->mode = mode_j.get<std::string>();

		const auto parse_soldier_list = [](std::vector<soldier_param_t>& list, nlohmann::json& data)
		{
			if (!data.is_array())
			{
				return;
			}

			for (auto i = 0ull; i < data.size(); i++)
			{
				auto& entry_j = data[i];
				if (!entry_j.is_object())
				{
					continue;
				}

				auto& param_j = entry_j["param"];
				if (!param_j.is_array() || param_j.size() != 2)
				{
					continue;
				}

				auto& param0_j = param_j[0];
				auto& param1_j = param_j[1];

				if (!param0_j.is_number_unsigned() || !param1_j.is_number_unsigned())
				{
					continue;
				}

				param_t::soldier_param_t param{};
				param.header = param0_j.get<std::uint32_t>();
				param.seed = param1_j.get<std::uint32_t>();
				list.emplace_back(param);
			}
		};

		parse_soldier_list(this->injure_soldier_id, injure_soldier_id_j);
		parse_soldier_list(this->capture_soldier_id, capture_soldier_id_j);
		parse_soldier_list(this->kill_soldier_id, kill_soldier_id_j);

		return true;
	}

	nlohmann::json cmd_send_sneak_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		
		param_t param{};
		context_t context{};

		if (!param.parse(data))
		{
			return error(ERR_INVALIDARG);
		}

		const auto _0 = gsl::finally([&]
		{
			database::players::abort_mother_base(player->get_id());
		});

		auto& event_j = data["event"];
		if (!parse_event_log(event_j, context.event_log) || !parse_event_data(event_j, context.event_data))
		{
			return error(ERR_INVALIDARG);
		}

		param.sneak_point = std::clamp(param.sneak_point, -500000, 500000);
		param.event_point = std::clamp(param.event_point, 0, 500000);
		context.is_win = param.sneak_result == "WIN";

		context.mode = database::players::get_sneak_mode_id(param.mode);
		if (context.mode == database::players::mode_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		context.active_sneak = database::players::find_active_sneak_from_player(player->get_id());
		if (!context.active_sneak.has_value() || context.active_sneak->get_status() < database::players::status_pre_game ||
			context.active_sneak->get_fob_id() != param.mother_base_id || context.active_sneak->get_mode() != context.mode)
		{
			return error(ERR_DATABASE);
		}

		context.attacker_data = database::player_data::find(player->get_id());
		context.fob = database::fobs::get_fob(context.active_sneak->get_fob_id());
		context.owner = database::players::find(context.fob->get_player_id());
		context.owner_data = database::player_data::find(context.owner->get_id());
		context.owner_record = database::player_records::find(context.owner->get_id());

		if (!context.attacker_data.has_value() || !context.fob.has_value() || 
			!context.owner.has_value() || !context.owner_data.has_value() || 
			!context.owner_record.has_value())
		{
			return error(ERR_DATABASE);
		}

		if (!context.owner->is_real_player())
		{
			this->handle_result_event(player.value(), param, context);
			result["is_wormhole_open"] = 0;
		}
		else
		{
			if (!this->handle_result(player.value(), param, context))
			{
				return error(ERR_DATABASE);
			}

			const auto wormhole = this->open_wormhole(player.value(), param, context);
			result["is_wormhole_open"] = std::uint8_t(wormhole);
		}

		this->handle_rewards(player.value(), param, context, result["result_reward"]);

		const auto new_stats = database::player_records::find(player->get_id());
		if (!new_stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		result["sneak_point"] = new_stats->get_fob_point();
		result["is_security_challenge"] = context.owner->is_security_challenge_enabled();
		result["event_point"] = new_stats->get_event_point();

		database::event_rankings::set_event_value(player->get_id(), database::event_rankings::ep_earned, new_stats->get_fob_point());

		return result;
	}

	bool cmd_send_sneak_result::needs_player()
	{
		return true;
	}
}
