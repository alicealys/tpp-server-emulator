#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/event_rankings.hpp"

namespace emulator::tpp
{
	namespace
	{
		struct soldier_param_t
		{
			std::uint32_t header;
			std::uint32_t seed;
		};

		enum soldier_array_action
		{
			soldier_kill = 0,
			soldier_injure = 1,
			soldier_capture = 2
		};

		using soldier_map_t = std::unordered_map<std::uint32_t, soldier_param_t>;

		soldier_map_t parse_soldier_id_list(nlohmann::json& list)
		{
			if (!list.is_array())
			{
				return {};
			}

			soldier_map_t soldier_ids;
			const auto count = std::min(256u, static_cast<std::uint32_t>(list.size()));
			for (auto i = 0ull; i < count; i++)
			{
				auto& soldier_param = list[i]["param"];
				if (soldier_param.size() != 2 || !soldier_param[0].is_number_unsigned() || !soldier_param[1].is_number_unsigned())
				{
					continue;
				}

				soldier_param_t param{};
				param.header = soldier_param[0].get<std::uint32_t>();
				param.seed = soldier_param[1].get<std::uint32_t>();
				soldier_ids.insert(std::make_pair(param.seed >> 11, param));
			}

			return soldier_ids;
		}

		void modify_staff_array(const std::uint64_t owner_id, database::player_data::staff_array_container& staff_array, 
			database::player_data::prisoner_array_container& attacker_prison,
			soldier_map_t& soldier_ids, const soldier_array_action action, std::uint32_t* counts)
		{
			auto prison_first_free = attacker_prison.get_first_free();
			const auto attack_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

			for (auto i = 0u; i < game::max_staff_count; i++)
			{
				if (soldier_ids.empty())
				{
					break;
				}

				auto staff = &staff_array[i];
				auto iter = soldier_ids.find(staff->fields.seed.data >> 11);
				if (iter == soldier_ids.end())
				{
					continue;
				}

				soldier_ids.erase(iter);

				switch (action)
				{
				case soldier_injure:
					counts[staff->fields.header.peak_rank]++;
					staff->fields.status_sync.health_state = 1;
					staff->fields.status_sync.designation = game::des_sickbay;
					break;
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
					break;
				}
				case soldier_kill:
					counts[staff->fields.header.peak_rank]++;
					std::memset(staff, 0, sizeof(game::staff_t));
					break;
				}
			}
		}

		void recover_captured_soldiers(
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
		}

		void write_staff(const database::player_data::player_data& player_data, const database::player_data::staff_array_container& staff_array)
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

			database::player_data::set_soldier_data(player_data.get_player_id(), new_staff_count, staff_array, levels, counts);
		}

		void update_staff(
			const database::player_data::player_data& owner_data,
			const database::player_data::player_data& attacker_data,
			nlohmann::json& data, 
			database::sneak_results::sneak_result_data_t& event_data,
			const bool has_insurance)
		{
			database::player_data::staff_array_container owner_staff;
			owner_data.get_staff_array(owner_staff);

			database::player_data::prisoner_array_container owner_prison;
			owner_data.get_prisoner_array(owner_prison);

			database::player_data::staff_array_container attacker_staff;
			attacker_data.get_staff_array(attacker_staff);

			database::player_data::prisoner_array_container attacker_prison;
			attacker_data.get_prisoner_array(attacker_prison);

			static std::vector<std::pair<std::string, soldier_array_action>> soldier_list_map =
			{
				{"capture_soldier_id", soldier_capture},
				{"injure_soldier_id", soldier_injure},
				{"kill_soldier_id", soldier_kill},
			};

			for (const auto& [name, action] : soldier_list_map)
			{
				auto list = parse_soldier_id_list(data[name]);
				auto counts = event_data.injury_staff_count;
				switch (action)
				{
				case soldier_capture:
					counts = event_data.capture_staff_count;
					break;
				case soldier_injure:
					counts = event_data.injury_staff_count;
					break;
				case soldier_kill:
					counts = event_data.kill_staff_count;
					break;
				}

				modify_staff_array(owner_data.get_player_id(), owner_staff, attacker_prison, list, action, counts);
			}

			const auto is_win = data["sneak_result"] == "WIN";
			if (is_win)
			{
				recover_captured_soldiers(attacker_data.get_player_id(), attacker_staff, owner_prison);
			}

			if (!has_insurance)
			{
				write_staff(owner_data, owner_staff);
			}

			write_staff(attacker_data, attacker_staff);

			database::player_data::set_prison_bin(owner_data.get_player_id(), owner_prison);
			database::player_data::set_prison_bin(attacker_data.get_player_id(), attacker_prison);
		}

		void update_resources(
			const database::player_data::player_data& attacker_data,
			const database::player_data::player_data& owner_data,
			nlohmann::json& data,
			database::sneak_results::sneak_result_data_t& sneak_data,
			const bool has_insurance)
		{
			database::player_data::resource_arrays_t owner_resources{};
			database::player_data::resource_arrays_t attacker_resources{};

			owner_data.get_resource_arrays(owner_resources);
			attacker_data.get_resource_arrays(attacker_resources);

			const auto do_resource = [&](nlohmann::json& from, const std::string& key, 
				const std::uint32_t resource_type, 
				const std::uint32_t resource_id, const std::uint32_t cap, bool destroy = false)
			{
				auto value = std::min(cap, from[key].get<std::uint32_t>());
				value = std::min(attacker_resources[resource_type][resource_id], value);
				if (!has_insurance)
				{
					owner_resources[resource_type][resource_id] -= value;
				}

				if (!destroy)
				{
					attacker_resources[resource_type][resource_id] += value;
				}

				return value;
			};

			constexpr const auto placement_cap = 4u * 4u;
			constexpr const auto resources_cap = 500000u;

			sneak_data.capture_nuclear = static_cast<std::uint8_t>(do_resource(data, "capture_nuclear", game::processed_server, game::nuclear, 4));
			sneak_data.capture_resource.biotic_resource = do_resource(data["capture_resource"], "biotic_resource", game::unprocessed_server, game::minor_metal, resources_cap);
			sneak_data.capture_resource.common_metal = do_resource(data["capture_resource"], "common_metal", game::unprocessed_server, game::common_metal, resources_cap);
			sneak_data.capture_resource.fuel_resource = do_resource(data["capture_resource"], "fuel_resource", game::unprocessed_server, game::fuel_resource, resources_cap);
			sneak_data.capture_resource.minor_metal = do_resource(data["capture_resource"], "minor_metal", game::unprocessed_server, game::precious_metal, resources_cap);
			sneak_data.capture_resource.precious_metal = do_resource(data["capture_resource"], "precious_metal", game::unprocessed_server, game::biotic_resource, resources_cap);

			sneak_data.capture_placement.mortar_normal = do_resource(data["capture_placement"], "mortar_normal", game::processed_server, game::mortar_normal, placement_cap);
			sneak_data.capture_placement.gatling_gun_east = do_resource(data["capture_placement"], "gatling_gun_east", game::processed_server, game::gatling_gun_east, placement_cap);
			sneak_data.capture_placement.gatling_gun_west = do_resource(data["capture_placement"], "gatling_gun_west", game::processed_server, game::gatling_gun_west, placement_cap);
			sneak_data.capture_placement.emplacement_gun_east = do_resource(data["capture_placement"], "emplacement_gun_east", game::processed_server, game::emplacement_gun_east, placement_cap);
			sneak_data.capture_placement.emplacement_gun_west = do_resource(data["capture_placement"], "emplacement_gun_west", game::processed_server, game::emplacement_gun_west, placement_cap);

			sneak_data.destroy_placement.mortar_normal = do_resource(data["destroy_placement"], "mortar_normal", game::processed_server, game::mortar_normal, placement_cap, true);
			sneak_data.destroy_placement.gatling_gun_east = do_resource(data["destroy_placement"], "gatling_gun_east", game::processed_server, game::gatling_gun_east, placement_cap, true);
			sneak_data.destroy_placement.gatling_gun_west = do_resource(data["destroy_placement"], "gatling_gun_west", game::processed_server, game::gatling_gun_west, placement_cap, true);
			sneak_data.destroy_placement.emplacement_gun_east = do_resource(data["destroy_placement"], "emplacement_gun_east", game::processed_server, game::emplacement_gun_east, placement_cap, true);
			sneak_data.destroy_placement.emplacement_gun_west = do_resource(data["destroy_placement"], "emplacement_gun_west", game::processed_server, game::emplacement_gun_west, placement_cap, true);
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
			const auto set_value = []<typename T>(nlohmann::json& v, T* ptr, const bool is_signed)
			{
				if ((is_signed && v.is_number_integer()) || (!is_signed && v.is_number_unsigned()))
				{
					*ptr = v.get<T>();
				}
			};

			set_value(event["gmp"], &event_data.gmp, true);
			set_value(event["layout_code"], &event_data.layout_code, false);
			set_value(event["position_x"], &event_data.position_x, true);
			set_value(event["position_z"], &event_data.position_z, true);
			set_value(event["rotate_y"], &event_data.rotate_y, true);
			set_value(event["cluster"], &event_data.cluster, false);

			return true;
		}
	}

	nlohmann::json cmd_send_sneak_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
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
			return error(ERR_DATABASE);
		}

		const auto attacker_data = database::player_data::find(player->get_id());
		if (!attacker_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto _0 = gsl::finally([&]
		{
			database::players::abort_mother_base(player->get_id());
		});

		const auto& sneak_result_j = data["sneak_result"];
		const auto& sneak_point_j = data["sneak_point"];
		const auto& event_point_j = data["event_point"];
		const auto& is_event_j = data["is_event"];
		auto& event_j = data["event"];
		const auto& mode_str_j = data["mode"];
		const auto& mother_base_id_j = data["mother_base_id"];

		if (!sneak_result_j.is_string() || !sneak_point_j.is_number() || !event_point_j.is_number_unsigned() ||
			!is_event_j.is_number_unsigned() || !mode_str_j.is_string() || !mother_base_id_j.is_number_unsigned() ||
			!event_j.is_object())
		{
			return error(ERR_INVALIDARG);
		}

		std::string event_log;
		if (!parse_event_log(event_j, event_log))
		{
			return error(ERR_INVALIDARG);
		}

		database::sneak_results::sneak_result_data_t event_data{};
		if (!parse_event_data(event_j, event_data))
		{
			return error(ERR_INVALIDARG);
		}

		const auto sneak_result = sneak_result_j.get<std::string>();
		const auto is_win = sneak_result == "WIN";
		const auto sneak_point = data["sneak_point"].get<std::int32_t>();
		const auto event_point = data["event_point"].get<std::uint32_t>();
		const auto is_event = data["is_event"].get<std::uint32_t>() == 1;
		const auto mode_str = data["mode"].get<std::string>();
		const auto mother_base_id = data["mother_base_id"].get<std::uint64_t>();

		const auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto owner = database::players::find(fob->get_player_id());
		if (!owner.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto mode = database::players::get_sneak_mode_id(mode_str);
		if (mode == database::players::mode_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		const auto send_update = [&]
		{
			const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());
			if (!active_sneak.has_value())
			{
				return ERR_DATABASE;
			}

			if (active_sneak->get_mode() != mode)
			{
				return ERR_DATABASE;
			}

			if (active_sneak->get_mode() == database::players::mode_actual)
			{
				const auto is_sneak = active_sneak->is_sneak();
				database::player_records::add_sneak_result(player->get_id(), fob->get_player_id(), sneak_point, is_win, is_sneak);

				if (sneak_point > 0)
				{
					database::event_rankings::set_value_if_bigger(player->get_id(), database::event_rankings::most_ep_in_mission, sneak_point);
				}

				if (is_sneak) // attack
				{
					if (!active_sneak->is_security_challenge())
					{
						database::player_records::set_shield_date(active_sneak->get_owner_id(), is_win);
					}

					auto deploy_damage_opt = attacker_data->get_fob_deploy_damage_param();
					if (deploy_damage_opt.has_value())
					{
						auto& deploy_damage = deploy_damage_opt.value();
						if (deploy_damage["motherbase_id"] == fob->get_id())
						{
							database::player_data::set_fob_deploy_damage_param(player->get_id(), {});
						}
					}

					if (is_win)
					{
						const auto defense_sneak = database::players::find_active_sneak(active_sneak->get_owner_id(), false, true);
						const auto event_id = defense_sneak.has_value()
							? database::event_rankings::cores_reached_defender
							: database::event_rankings::cores_reached_no_defender;
						database::event_rankings::increment_event_value(player->get_id(), event_id, 1);
					}

					auto owner_data = database::player_data::find(owner->get_id());
					const auto owner_record = database::player_records::find(owner->get_id());
					const auto owner_has_insurance = owner_record.has_value() && owner_record->get_is_insurance();

					if (!database::vars.pvp_mode && !database::vars.no_fob_damage && owner_data.has_value() && attacker_data.has_value())
					{
						update_staff(owner_data.value(), attacker_data.value(), data, event_data, owner_has_insurance);
						update_resources(owner_data.value(), attacker_data.value(), data, event_data, owner_has_insurance);
					}

					auto& active_sneak_val = active_sneak.value();
					if (!database::sneak_results::add_sneak_result(player.value(), fob.value(), active_sneak_val, is_win, event_data, event_log))
					{
						result["result"] = game::get_error(ERR_DATABASE);
					}
				}
				else // defense
				{
					const auto& count_of_neutralized_by_intruder_j = data["count_of_neutralized_by_intruder"];
					if (count_of_neutralized_by_intruder_j.is_number_unsigned())
					{
						const auto c = std::min(100u, count_of_neutralized_by_intruder_j.get<std::uint32_t>());
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::times_neutralized_by_intruder, c);
					}

					const auto& count_of_neutralize_intruder_j = data["count_of_neutralize_intruder"];
					if (count_of_neutralize_intruder_j.is_number_unsigned() && player->get_id() == active_sneak->get_owner_id())
					{
						const auto c = std::min(100u, count_of_neutralize_intruder_j.get<std::uint32_t>());
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::defender_neutralized_intruder, c);
					}

					if (is_win)
					{
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::successful_defenses, 1);
					}

					if (active_sneak->get_owner_id() == player->get_id())
					{
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::defense_deployments, 1);
					}
					else
					{
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::defense_deployments_support, 1);
						database::event_rankings::increment_event_value(active_sneak->get_owner_id(), database::event_rankings::defense_deployments_support_received, 1);
					}
				}
			}

			return NOERR;
		};

		if (is_event && database::fob_events::is_event_player(owner->get_id()))
		{
			database::player_records::add_event_points(player->get_id(), event_point);
			database::event_rankings::set_value_if_bigger(player->get_id(), database::event_rankings::most_ep_in_mission, sneak_point);
			database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::fob_event_ranking, event_point);
			database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::cores_reached_no_defender, 1);
			database::player_records::add_sneak_result(player->get_id(), fob->get_player_id(), sneak_point, is_win, true);
		}

		if (owner->is_real_player())
		{
			const auto err = send_update();
			if (err != NOERR)
			{
				return error(err);
			}
		}

		const auto new_stats = database::player_records::find(player->get_id());
		if (!new_stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		result["sneak_point"] = new_stats->get_fob_point();
		result["is_security_challenge"] = 0;
		result["is_wormhole_open"] = 0;
		result["event_point"] = new_stats->get_event_point();

		return result;
	}
}
