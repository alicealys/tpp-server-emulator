#include <std_include.hpp>

#include "cmd_sneak_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"
#include "database/models/fob_prisoners.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_sneak_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		auto& is_sneak_j = data["is_sneak"];
		auto& is_event_j = data["is_event"];
		auto& is_plus_j = data["is_plus"];
		auto& mode_j = data["mode"];
		auto& mother_base_id_j = data["mother_base_id"];
		auto& platform_j = data["platform"];
		auto& player_id_j = data["player_id"];
		auto& wormhole_player_id_j = data["wormhole_player_id"];

		if (!is_sneak_j.is_number_integer() || !is_event_j.is_number_integer() || !is_plus_j.is_number_integer() || 
			!mode_j.is_string() || !mother_base_id_j.is_number_integer() || !platform_j.is_number_unsigned() || 
			!player_id_j.is_number_unsigned() || !wormhole_player_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_sneak = is_sneak_j.get<std::uint32_t>() == 1;
		const auto is_event = is_event_j.get<std::uint32_t>() == 1;
		const auto mode_str = mode_j.get<std::string>();

		const auto mode = database::players::get_sneak_mode_id(mode_str);
		const auto alt_mode = database::players::get_alt_sneak_mode(mode);

		if (mode == database::players::mode_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		auto fob = database::fobs::get_fob(mother_base_id);

		if (!fob.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		auto attacker_id = player->get_id();
		if (!is_sneak)
		{
			const auto attacker_sneak = database::players::find_active_sneak(fob->get_player_id(), mode, alt_mode, true);
			if (!attacker_sneak.has_value())
			{
				return error(ERR_DATABASE);
			}

			attacker_id = attacker_sneak->get_player_id();
		}

		const auto attacker_data = database::player_data::find(attacker_id);
		const auto owner_data = database::player_data::find(fob->get_player_id());

		if (!attacker_data.has_value() || !owner_data.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto owner = database::players::find(fob->get_player_id());
		if (!owner.has_value())
		{
			return error(ERR_DATABASE);
		}

		game::motherbase_t motherbase{};
		owner_data->get_motherbase(motherbase);

		const auto active_sneak = database::players::get_active_sneak(mother_base_id);
		auto platform = platform_j.get<std::uint32_t>();
		if (active_sneak.has_value())
		{
			platform = active_sneak->get_platform();
		}

		auto& cluster_param = fob->get_cluster_param();
		if (platform >= game::fob_clusters_count)
		{
			return error(ERR_INVALIDARG);
		}

		if (owner->is_real_player())
		{
			const auto current_sneak = database::players::find_active_sneak(fob->get_player_id(), mode, alt_mode, is_sneak);
			if (!current_sneak.has_value())
			{
				return error(ERR_DATABASE);
			}

			const auto sneak_player_id = current_sneak->get_player_id();
			if (player->get_id() != sneak_player_id || fob->get_id() != current_sneak->get_fob_id())
			{
				return error(ERR_DATABASE);
			}
		}

		result["damage_param"] = nlohmann::json::array();
		result["event_fob_params"] = {0, 0, 0, 0, 0};

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

		result["is_event"] = std::uint8_t(is_event);
		result["is_security_contract"] = 0;
		result["owner_gmp"] = owner_data->get_server_gmp();

		game::fob_resources_t recover_resource{};
		recover_resource.to_json(result["recover_resource"]);

		if (owner->is_real_player() && current_deploy.has_value() && current_deploy->get_fob_id() == fob->get_id())
		{
			current_deploy->apply(cluster_param);
		}

		{
			auto& recover_soldier_j = result["recover_soldier"];
			recover_soldier_j = nlohmann::json::array();
			std::array<std::uint32_t, 10> recover_soldier_count{};

			const auto rescuable_list = database::fob_prisoners::get_rescueable_list(player->get_id(), owner->get_id());
			for (auto i = 0ull; i < rescuable_list.size(); i++)
			{
				const auto param = rescuable_list[i].get_staff_param();
				auto& entry_j = recover_soldier_j[i];
				entry_j["header"] = param.packed_header;
				entry_j["seed"] = param.packed_seed;
				entry_j["status_no_sync"] = param.packed_status_no_sync;
				entry_j["status_sync"] = param.packed_status_sync;

				const auto peak_rank = param.header.peak_rank;
				recover_soldier_count[peak_rank]++;
			}

			result["recover_soldier_count"] = recover_soldier_count;
			result["recover_soldier_num"] = rescuable_list.size();
		}

		database::player_data::staff_array_container staff_array;
		owner_data->get_staff_array(staff_array);

		result["reward_id"] = 0;
		result["reward_soldier"] = nlohmann::json::array();
		result["reward_soldier_num"] = 0;
		result["reward_soldier_rank"] = 0;
		result["reward_soldier_type"] = 0;

		auto& stage_param = result["stage_param"];

		result["security_soldier"] = nlohmann::json::array();
		result["security_soldier_num"] = 0;
		result["security_soldier_rank"] = 0;

		const auto cluster_type = game::cluster_index_map[platform];
		const auto& mapped_cluster_param = cluster_param.param[cluster_type];

		const auto matches_current_platform = [&](const game::staff_t& staff)
		{
			switch (staff.fields.status_sync.designation)
			{
			case game::SECTION_COMBAT:
				return cluster_type == game::CLUSTER_CATEGORY_COMBAT || cluster_type == game::CLUSTER_CATEGORY_COMMAND;
			case game::SECTION_DEVELOP:
				return cluster_type == game::CLUSTER_CATEGORY_DEVELOP || cluster_type == game::CLUSTER_CATEGORY_COMMAND;
			case game::SECTION_BASE_DEV:
				return cluster_type == game::CLUSTER_CATEGORY_BASE_DEV || cluster_type == game::CLUSTER_CATEGORY_COMMAND;
			case game::SECTION_SUPPORT:
				return cluster_type == game::CLUSTER_CATEGORY_SUPPORT || cluster_type == game::CLUSTER_CATEGORY_COMMAND;
			case game::SECTION_SPY:
				return cluster_type == game::CLUSTER_CATEGORY_SPY || cluster_type == game::CLUSTER_CATEGORY_COMMAND;
			case game::SECTION_MEDICAL:
				return cluster_type == game::CLUSTER_CATEGORY_MEDICAL || cluster_type == game::CLUSTER_CATEGORY_COMMAND;
			}

			return false;
		};

		if (is_event)
		{
			result["security_soldier_rank"] = mapped_cluster_param.soldier_rank;
			auto security_soldier_num = 0u;

			security_soldier_num += mapped_cluster_param.unique_security.soldier;
			security_soldier_num += mapped_cluster_param.common_security[0].soldier;
			security_soldier_num += mapped_cluster_param.common_security[1].soldier;
			security_soldier_num += mapped_cluster_param.common_security[2].soldier;

			result["security_soldier_num"] = security_soldier_num;
		}

		auto security_soldier_index = 0;
		auto reward_soldier_index = 0;

		for (auto i = 0u; i < owner_data->get_staff_count(); i++)
		{
			auto& staff = staff_array[i];
			if (!game::is_usable_staff(staff))
			{
				continue;
			}

			if (staff.fields.packed_status_no_sync == 0)
			{
				staff.fields.packed_status_no_sync = 4096; // fix broken staff
			}

			if (!is_event && staff.fields.status_sync.designation == game::SECTION_SECURITY)
			{
				result["security_soldier"][security_soldier_index]["header"] = staff.fields.packed_header;
				result["security_soldier"][security_soldier_index]["seed"] = staff.fields.packed_seed;
				result["security_soldier"][security_soldier_index]["status_no_sync"] = staff.fields.packed_status_no_sync;
				result["security_soldier"][security_soldier_index]["status_sync"] = staff.fields.packed_status_sync;
				++security_soldier_index;
			}
			else if (matches_current_platform(staff) && reward_soldier_index < 10)
			{
				result["reward_soldier"][reward_soldier_index]["header"] = staff.fields.packed_header;
				result["reward_soldier"][reward_soldier_index]["seed"] = staff.fields.packed_seed;
				result["reward_soldier"][reward_soldier_index]["status_no_sync"] = staff.fields.packed_status_no_sync;
				result["reward_soldier"][reward_soldier_index]["status_sync"] = staff.fields.packed_status_sync;
				++reward_soldier_index;
			}
		}

		if (!is_event)
		{
			result["security_soldier_num"] = security_soldier_index;
		}

		result["reward_soldier_num"] = reward_soldier_index;

		mapped_cluster_param.to_json(stage_param["cluster_param"]);

		for (auto i = 0ull; i < game::fob_clusters_count; i++)
		{
			stage_param["build"][i] = cluster_param.param[i].build.packed;
		}

		stage_param["construct_param"] = fob->get_construct_param().packed;
		stage_param["fob_index"] = fob->get_index();
		stage_param["mother_base_id"] = fob->get_id();
		stage_param["nuclear"] = owner_data->get_nuke_count();
		stage_param["owner_player_id"] = fob->get_player_id();
		stage_param["platform"] = platform_j;
		stage_param["equip_grade"] = motherbase.equip_grade;
		stage_param["security_level"] = motherbase.security_level;

		database::player_data::resource_arrays_t resource_arrays{};
		owner_data->get_resource_arrays(resource_arrays);

		game::fob_placements_t emplacements{};
		emplacements.emplacement_gun_east = resource_arrays[game::processed_server][game::EMPLACEMENT_GUN_EAST];
		emplacements.emplacement_gun_west = resource_arrays[game::processed_server][game::EMPLACEMENT_GUN_WEST];
		emplacements.gatling_gun_east = resource_arrays[game::processed_server][game::ANTI_AIR_GATLING_GUN_EAST];
		emplacements.gatling_gun_west = resource_arrays[game::processed_server][game::ANTI_AIR_GATLING_GUN_WEST];
		emplacements.mortar_normal = resource_arrays[game::processed_server][game::MORTAR_NORMAL];
		emplacements.to_json(stage_param["placement"]);

		game::fob_resources_t processing_resource{};
		processing_resource.biotic_resource = resource_arrays[game::unprocessed_server][game::BIOTIC_RESOURCE];
		processing_resource.common_metal = resource_arrays[game::unprocessed_server][game::COMMON_METAL];
		processing_resource.minor_metal = resource_arrays[game::unprocessed_server][game::MINOR_METAL];
		processing_resource.fuel_resource = resource_arrays[game::unprocessed_server][game::FUEL_RESOURCE];
		processing_resource.precious_metal = resource_arrays[game::unprocessed_server][game::PRECIOUS_METAL];
		processing_resource.to_json(stage_param["processing_resource"]);

		game::fob_resources_t usable_resource{};
		processing_resource.biotic_resource = resource_arrays[game::processed_server][game::BIOTIC_RESOURCE];
		processing_resource.common_metal = resource_arrays[game::processed_server][game::COMMON_METAL];
		processing_resource.minor_metal = resource_arrays[game::processed_server][game::MINOR_METAL];
		processing_resource.fuel_resource = resource_arrays[game::processed_server][game::FUEL_RESOURCE];
		processing_resource.precious_metal = resource_arrays[game::processed_server][game::PRECIOUS_METAL];
		usable_resource.to_json(stage_param["usable_resource"]);

		auto& section_level_j = stage_param["section_level"];
		section_level_j["combat"] = owner_data->unit_levels.value[game::unit_combat];
		section_level_j["base"] = owner_data->unit_levels.value[game::unit_base_dev];
		section_level_j["develop"] = owner_data->unit_levels.value[game::unit_rnd];
		section_level_j["spy"] = owner_data->unit_levels.value[game::unit_intel];
		section_level_j["suport"] = owner_data->unit_levels.value[game::unit_support];
		section_level_j["medical"] = owner_data->unit_levels.value[game::unit_medical];
		section_level_j["security"] = owner_data->unit_levels.value[game::unit_security];

		result["wormhole_player_id"] = wormhole_player_id_j;

		if (owner->is_real_player())
		{
			if (!is_sneak)
			{
				if (!active_sneak.has_value())
				{
					return error(ERR_DATABASE);
				}

				stage_param["platform"] = active_sneak->get_platform();

				if (!database::players::set_active_sneak(player->get_id(), fob->get_id(), fob->get_player_id(), active_sneak->get_platform(), mode,
					database::players::status_pre_game, is_sneak, owner->is_security_challenge_enabled()))
				{
					return error(ERR_DATABASE);
				}
			}
			else
			{
				database::player_records::clear_shield_date(player->get_id());

				if (!database::players::set_active_sneak(player->get_id(), fob->get_id(), fob->get_player_id(), platform, mode,
					database::players::status_pre_game, is_sneak, owner->is_security_challenge_enabled()))
				{
					return error(ERR_DATABASE);
				}
			}
		}

		return result;
	}

	bool cmd_sneak_mother_base::needs_player()
	{
		return true;
	}
}
