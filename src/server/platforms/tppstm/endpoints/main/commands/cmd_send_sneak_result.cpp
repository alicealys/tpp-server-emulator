#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"
#include "database/models/fob_events.hpp"
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

		struct soldier_param_t
		{
			std::uint32_t header;
			std::uint32_t seed;
			bool valid;
		};

		using soldier_map_t = std::unordered_map<std::uint32_t, soldier_param_t>;

		soldier_map_t make_soldier_map(const std::vector<cmd_send_sneak_result::param_t::soldier_param_t>& list)
		{
			soldier_map_t map;

			for (const auto& entry : list)
			{
				soldier_param_t param{};
				param.header = entry.header;
				param.seed = entry.seed & 0xFFFFF800;
				param.valid = true;
				map.insert(std::make_pair(param.seed, param));
			}

			return map;
		}

		bool modify_staff_array(const std::uint64_t owner_id, database::player_data::staff_array_container& staff_array, 
			database::player_data::prisoner_array_container& attacker_prison,
			soldier_map_t& soldier_ids, const soldier_array_action action, std::uint32_t* counts)
		{
			std::int64_t prison_first_free = -1;
			if (action == soldier_capture)
			{
				prison_first_free = attacker_prison.get_first_free();
			}

			auto result = false;
			const auto attack_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			auto soldiers_left = soldier_ids.size();

			for (auto i = 0u; i < game::max_staff_count; i++)
			{
				if (soldiers_left <= 0)
				{
					break;
				}

				auto staff = &staff_array[i];
				const auto seed = staff->fields.seed.data & 0xFFFFF800;
				auto iter = soldier_ids.find(seed);
				if (iter == soldier_ids.end() || !iter->second.valid)
				{
					continue;
				}

				soldiers_left--;
				iter->second.valid = false;

				if (staff->fields.status_sync.direct_contract)
				{
					continue;
				}

				switch (action)
				{
				case soldier_injure:
				{
					counts[staff->fields.header.peak_rank]++;
					staff->fields.status_sync.health_state = 1;
					staff->fields.status_sync.designation = game::des_sickbay;
					result = true;
					break;
				}
				case soldier_capture:
				{
					counts[staff->fields.header.peak_rank]++;
					if (prison_first_free != -1 && prison_first_free < static_cast<std::int64_t>(attacker_prison.size()))
					{
						auto& prisoner = attacker_prison[prison_first_free++];
						prisoner.owner_id = owner_id;
						prisoner.time_captured = attack_time;
						std::memcpy(&prisoner.data, staff, sizeof(game::staff_t));
					}
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

		bool recover_captured_soldiers(
			std::uint64_t attacker_id,
			database::player_data::staff_array_container& attacker_staff, 
			database::player_data::prisoner_array_container& owner_prison)
		{
			database::player_data::prisoner_array_container new_prison;
			std::queue<game::staff_t> staff_recovered;

			auto new_prison_size = 0;

			for (auto i = 0u; i < owner_prison.size(); i++)
			{
				if (database::player_data::can_recover_prisoner(owner_prison[i]) && 
					owner_prison[i].owner_id == attacker_id)
				{
					staff_recovered.push(owner_prison[i].data);
				}
				else
				{
					new_prison[new_prison_size++] = owner_prison[i];
				}
			}

			if (staff_recovered.empty())
			{
				return false;
			}

			for (auto i = 0u; i < game::max_staff_count; i++)
			{
				if (staff_recovered.empty())
				{
					break;
				}

				if (attacker_staff[i].fields.packed_header == 0)
				{
					auto& staff = staff_recovered.back();
					staff_recovered.pop();
					std::memcpy(&attacker_staff[i], &staff, sizeof(game::staff_t));
				}
			}

			owner_prison = new_prison;
			return true;
		}

		void check_disable_security_challenge(const database::player_data::player_data& player_data, database::player_data::unit_counts_t& unit_counts)
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

			if (unit_counts[game::unit_security] < min_guards)
			{
				database::players::set_security_challenge(player_data.get_player_id(), false);
			}
		}

		void write_staff(const database::player_data::player_data& player_data, 
			const database::player_data::staff_array_container& staff_array, bool is_owner)
		{
			database::player_data::unit_counts_t counts{};
			database::player_data::unit_levels_t levels{};

			auto new_staff_count = 0;
			for (auto i = 0u; i < game::max_staff_count; i++)
			{
				const auto staff = &staff_array[i];
				if (staff->fields.status_sync.designation != 0)
				{
					new_staff_count++;

					if (staff->fields.status_sync.designation >= game::des_units_start &&
						staff->fields.status_sync.designation < game::des_units_end)
					{
						counts[staff->fields.status_sync.designation - game::des_units_start]++;
					}
				}
			}

			for (auto i = 0; i < game::unit_count; i++)
			{
				levels[i] = player_data.get_unit_level(i);
			}

			if (is_owner)
			{
				check_disable_security_challenge(player_data, counts);
			}

			database::player_data::set_soldier_data(player_data.get_player_id(), new_staff_count, staff_array, levels, counts);
		}

		void update_staff(
			const cmd_send_sneak_result::param_t& param,
			cmd_send_sneak_result::context_t& context,
			const bool has_insurance)
		{
			database::player_data::staff_array_container owner_staff;
			context.owner_data->get_staff_array(owner_staff);

			database::player_data::prisoner_array_container owner_prison;
			context.owner_data->get_prisoner_array(owner_prison);

			database::player_data::staff_array_container attacker_staff;
			context.attacker_data->get_staff_array(attacker_staff);

			database::player_data::prisoner_array_container attacker_prison;
			context.attacker_data->get_prisoner_array(attacker_prison);

			auto modified_owner_staff = false;
			if (!param.capture_soldier_id.empty())
			{
				auto capture_soldier_id = make_soldier_map(param.capture_soldier_id);
				modified_owner_staff |= modify_staff_array(context.owner_data->get_player_id(), owner_staff, attacker_prison,
					capture_soldier_id, soldier_capture, context.event_data.capture_staff_count);
			}

			if (!param.kill_soldier_id.empty())
			{
				auto kill_soldier_id = make_soldier_map(param.kill_soldier_id);
				if (modify_staff_array(context.owner_data->get_player_id(), owner_staff, attacker_prison,
					kill_soldier_id, soldier_kill, context.event_data.kill_staff_count))
				{
					modified_owner_staff = true;
					database::player_data::set_prison_bin(context.attacker_data->get_player_id(), attacker_prison);
				}
			}

			if (!param.injure_soldier_id.empty())
			{
				auto injury_staff_count = make_soldier_map(param.injure_soldier_id);
				modified_owner_staff |= modify_staff_array(context.owner_data->get_player_id(), owner_staff, attacker_prison,
					injury_staff_count, soldier_injure, context.event_data.injury_staff_count);
			}

			if (context.is_win && recover_captured_soldiers(context.attacker_data->get_player_id(), attacker_staff, owner_prison))
			{
				write_staff(context.attacker_data.value(), attacker_staff, false);
				database::player_data::set_prison_bin(context.owner_data->get_player_id(), owner_prison);
			}

			if (modified_owner_staff && !has_insurance)
			{
				write_staff(context.owner_data.value(), owner_staff, true);
			}
		}

		void update_resources(
			const cmd_send_sneak_result::param_t& param,
			cmd_send_sneak_result::context_t& context,
			const bool has_insurance)
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
				
				if (!has_insurance || resource_id == game::NUCLEAR_WEAPON)
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
			if (!event.is_object() || !event["data"].is_string() || !event["size"].is_number_unsigned())
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
		if (!context.active_sneak->is_sneak())
		{
			return;
		}

		const auto platform = context.active_sneak->get_platform();
		const auto& clusters = context.fob->get_cluster_param();
		const auto cluster_type = game::cluster_index_map[platform];
		const auto cluster = clusters.param[cluster_type];

		database::sneak_results::sneak_result_rewards_t reward_info{};
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
		context.active_sneak = database::players::find_active_sneak_from_player(player.get_id());
		if (!context.active_sneak.has_value())
		{
			return false;
		}

		if (context.active_sneak->get_mode() != context.mode)
		{
			return false;
		}

		if (context.active_sneak->get_mode() != database::players::mode_actual)
		{
			return true;
		}

		const auto is_sneak = context.active_sneak->is_sneak();
		database::player_records::add_sneak_result(player.get_id(), context.fob->get_player_id(),
			param.sneak_point, context.is_win, is_sneak);

		if (param.sneak_point > 0)
		{
			database::event_rankings::set_value_if_bigger(player.get_id(),
				database::event_rankings::most_ep_in_mission, param.sneak_point);
		}

		if (is_sneak) // attack
		{
			if (!context.active_sneak->is_security_challenge())
			{
				database::player_records::set_shield_date(context.active_sneak->get_owner_id(), context.is_win);
			}

			database::fob_deploy_assists::delete_deployments(player.get_id());

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

			auto owner_data = database::player_data::find(context.owner->get_id());
			const auto owner_record = database::player_records::find(context.owner->get_id());
			const auto owner_has_insurance = owner_record.has_value() && owner_record->get_is_insurance();

			if (!database::vars.pvp_mode && !database::vars.no_fob_damage && owner_data.has_value() && context.attacker_data.has_value())
			{
				update_staff(param, context, owner_has_insurance);
				update_resources(param, context, owner_has_insurance);
			}

			auto& active_sneak_val = context.active_sneak.value();
			if (!database::sneak_results::add_sneak_result(player, context.fob.value(),
				active_sneak_val, context.is_win, context.event_data, context.event_log))
			{
				return false;
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

		return true;
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

		context.attacker_data = database::player_data::find(player->get_id());
		if (!context.attacker_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto _0 = gsl::finally([&]
		{
			database::players::abort_mother_base(player->get_id());
		});

		auto& event_j = data["event"];
		if (!parse_event_log(event_j, context.event_log))
		{
			return error(ERR_INVALIDARG);
		}

		if (!parse_event_data(event_j, context.event_data))
		{
			return error(ERR_INVALIDARG);
		}

		param.sneak_point = std::clamp(param.sneak_point, -500000, 500000);
		param.event_point = std::clamp(param.event_point, 0, 500000);
		context.is_win = param.sneak_result == "WIN";

		context.fob = database::fobs::get_fob(param.mother_base_id);
		if (!context.fob.has_value())
		{
			return error(ERR_DATABASE);
		}

		context.owner = database::players::find(context.fob->get_player_id());
		if (!context.owner.has_value())
		{
			return error(ERR_DATABASE);
		}

		context.owner_data = database::player_data::find(context.owner->get_id());
		if (!context.owner_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		context.mode = database::players::get_sneak_mode_id(param.mode);
		if (context.mode == database::players::mode_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		if (!context.owner->is_real_player())
		{
			if (param.is_event)
			{
				database::player_records::add_event_points(player->get_id(), param.event_point);
				database::event_rankings::set_value_if_bigger(player->get_id(), database::event_rankings::most_ep_in_mission, param.sneak_point);
				database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::fob_event_ranking, param.event_point);
				database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::cores_reached_no_defender, 1);
				database::player_records::add_sneak_result(player->get_id(), context.fob->get_player_id(), param.sneak_point, context.is_win, true);
			}
		}
		else
		{
			if (!this->handle_result(player.value(), param, context))
			{
				return error(ERR_DATABASE);
			}

			this->handle_rewards(player.value(), param, context, result["result_reward"]);
		}

		const auto new_stats = database::player_records::find(player->get_id());
		if (!new_stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto wormhole = database::wormholes::get_wormhole_status(context.owner->get_id(), player->get_id());

		result["sneak_point"] = new_stats->get_fob_point();
		result["is_security_challenge"] = context.owner->is_security_challenge_enabled();
		result["is_wormhole_open"] = std::uint8_t(wormhole.open);
		result["event_point"] = new_stats->get_event_point();

		database::event_rankings::set_event_value(player->get_id(), database::event_rankings::ep_earned, new_stats->get_fob_point());

		return result;
	}

	bool cmd_send_sneak_result::needs_player()
	{
		return true;
	}
}
