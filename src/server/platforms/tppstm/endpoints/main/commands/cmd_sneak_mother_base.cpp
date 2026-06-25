#include <std_include.hpp>

#include "cmd_sneak_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_sneak_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
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

		const auto& is_sneak_j = data["is_sneak"];
		const auto& is_event_j = data["is_event"];
		const auto& is_plus_j = data["is_plus"];
		const auto& mode_j = data["mode"];
		const auto& mother_base_id_j = data["mother_base_id"];
		const auto& platform_j = data["platform"];
		const auto& player_id_j = data["player_id"];
		const auto& wormhole_player_id_j = data["wormhole_player_id"];

		if (!is_sneak_j.is_number_integer() || !is_event_j.is_number_integer() || !is_plus_j.is_number_integer()
			|| !mode_j.is_string() || !mother_base_id_j.is_number_integer() || !platform_j.is_number_unsigned()
			|| !player_id_j.is_number_unsigned() || !wormhole_player_id_j.is_number_unsigned())
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
		if (platform >= game::fob_sections_count)
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

		auto damage_params = attacker_data->get_fob_deploy_damage_param();
		if (damage_params.has_value())
		{
			result["fob_deploy_damage_param"] = damage_params.value();
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

		result["is_event"] = is_event ? 1 : 0;
		result["is_security_contract"] = 0;

		result["owner_gmp"] = owner_data->get_server_gmp();

		result["recover_resource"]["biotic_resource"] = 0;
		result["recover_resource"]["common_metal"] = 0;
		result["recover_resource"]["fuel_resource"] = 0;
		result["recover_resource"]["minor_metal"] = 0;
		result["recover_resource"]["precious_metal"] = 0;

		if (owner->is_real_player())
		{
			database::fobs::apply_deploy_damage_params(fob->get_id(), cluster_param, damage_params);
		}

		database::player_data::prisoner_array_container prison;
		owner_data->get_prisoner_array(prison);

		result["recover_soldier"] = nlohmann::json::array();
		result["recover_soldier_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		result["recover_soldier_num"] = 0;

		auto recover_soldier_index = 0;
		for (auto i = 0u; i < prison.size(); i++)
		{
			if (can_recover_prisoner(prison[i]) && prison[i].owner_id == player->get_id())
			{
				result["recover_soldier"][recover_soldier_index]["header"] = prison[i].data.fields.packed_header;
				result["recover_soldier"][recover_soldier_index]["seed"] = prison[i].data.fields.packed_seed;
				result["recover_soldier"][recover_soldier_index]["status_no_sync"] = prison[i].data.fields.packed_status_no_sync;
				result["recover_soldier"][recover_soldier_index]["status_sync"] = prison[i].data.fields.packed_status_sync;

				const auto peak_rank = prison[i].data.fields.header.peak_rank;
				auto& count_j = result["recover_soldier_count"][peak_rank];
				const auto count = count_j.get<std::uint32_t>();
				count_j = count + 1;
				++recover_soldier_index;
			}
		}

		result["recover_soldier_num"] = recover_soldier_index;

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

		const auto mapped_index = game::cluster_index_map[platform];
		const auto& mapped_cluster_param = cluster_param.param[mapped_index];

		const auto matches_current_platform = [&](const game::staff_t& staff)
		{
			if (platform == 0)
			{
				return true;
			}

			return platform == staff.fields.status_sync.designation;
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

			if (!is_event && staff.fields.status_sync.designation == game::des_security)
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

		stage_param["cluster_param"] = mapped_cluster_param.to_json();
		stage_param["build"] = {0, 0, 0, 0, 0, 0, 0};

		for (auto i = 0ull; i < game::fob_sections_count; i++)
		{
			stage_param["build"][i] = cluster_param.param[i].build.packed;
		}

		stage_param["construct_param"] = fob->get_construct_param().packed;

		stage_param["fob_index"] = fob->get_index();
		stage_param["mother_base_id"] = fob->get_id();
		stage_param["nuclear"] = owner_data->get_nuke_count();
		stage_param["owner_player_id"] = fob->get_player_id();

		database::player_data::resource_arrays_t resource_arrays{};
		owner_data->get_resource_arrays(resource_arrays);

		stage_param["placement"]["emplacement_gun_east"] = resource_arrays[game::processed_server][game::emplacement_gun_east];
		stage_param["placement"]["emplacement_gun_west"] = resource_arrays[game::processed_server][game::emplacement_gun_west];
		stage_param["placement"]["gatling_gun"] = 0;
		stage_param["placement"]["gatling_gun_east"] = resource_arrays[game::processed_server][game::gatling_gun_east];
		stage_param["placement"]["gatling_gun_west"] = resource_arrays[game::processed_server][game::gatling_gun_west];
		stage_param["placement"]["mortar_normal"] = resource_arrays[game::processed_server][game::mortar_normal];

		stage_param["platform"] = platform_j;
		stage_param["equip_grade"] = motherbase.equip_grade;
		stage_param["security_level"] = motherbase.security_level;

		static std::vector<std::string> material_resource_names =
		{
			"fuel_resource",
			"biotic_resource" ,
			"common_metal",
			"minor_metal",
			"precious_metal"
		};

		for (auto i = 0; i < 5; i++)
		{
			stage_param["processing_resource"][material_resource_names[i]] = resource_arrays[game::unprocessed_server][i];
		}

		for (auto i = 0; i < game::unit_count; i++)
		{
			stage_param["section_level"][game::unit_names[i]] = owner_data->get_unit_level(i);
		}

		for (auto i = 0; i < 5; i++)
		{
			stage_param["usable_resource"][material_resource_names[i]] = resource_arrays[game::processed_server][i];
		}

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
}
