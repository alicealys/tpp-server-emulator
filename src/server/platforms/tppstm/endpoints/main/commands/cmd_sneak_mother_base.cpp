#include <std_include.hpp>

#include "cmd_sneak_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sneak_mother_base::execute(nlohmann::json& data, const std::string& session_key)
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
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto is_sneak = is_sneak_j.get<std::uint32_t>() == 1;
		const auto mode_str = mode_j.get<std::string>();

		const auto mode = database::players::get_sneak_mode_id(mode_str);
		const auto alt_mode = database::players::get_alt_sneak_mode(mode);

		if (mode == database::players::mode_invalid)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto platform = platform_j.get<std::uint32_t>();

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		auto fob = database::fobs::get_fob(mother_base_id);
		const auto fob_list = database::fobs::get_fob_list(fob->get_player_id());
		if (!fob.has_value() || fob_list.size() == 0)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		auto fob_index = 0;
		for (auto i = 0; i < fob_list.size(); i++)
		{
			if (fob_list[i].get_id() == fob->get_id())
			{
				fob_index = i;
			}
		}

		const auto player_data = database::player_data::find(fob->get_player_id(), true);
		auto mother_base = player_data->get_motherbase();

		const auto& cluster_param = fob->get_cluster_param();
		if (platform >= cluster_param.size())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto current_sneak = database::players::find_active_sneak(fob->get_player_id(), mode, alt_mode, is_sneak);
		if (!current_sneak.has_value())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		const auto sneak_player_id = current_sneak->get_player_id();
		if (player->get_id() != sneak_player_id || fob->get_id() != current_sneak->get_fob_id())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		result["damage_param"] = nlohmann::json::array();
		result["event_fob_params"] = {0, 0, 0, 0, 0};

		result["fob_deploy_damage_param"]["cluster_index"] = 0;
		result["fob_deploy_damage_param"]["expiration_date"] = 0;
		result["fob_deploy_damage_param"]["motherbase_id"] = 0;

		for (auto i = 0; i < 16; i++)
		{
			result["fob_deploy_damage_param"]["damage_values"][i] = 0;
		}

		result["is_event"] = 0;
		result["is_security_contract"] = 0;

		result["owner_gmp"] = player_data->get_server_gmp();

		result["recover_resource"]["biotic_resource"] = 0;
		result["recover_resource"]["common_metal"] = 0;
		result["recover_resource"]["fuel_resource"] = 0;
		result["recover_resource"]["minor_metal"] = 0;
		result["recover_resource"]["precious_metal"] = 0;

		result["recover_soldier"] = nlohmann::json::array();
		result["recover_soldier_count"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		result["recover_soldier_num"] = 0;
		result["reward_id"] = 0;
		result["reward_soldier"] = nlohmann::json::array();

		result["reward_soldier_num"] = 0;
		result["reward_soldier_rank"] = 0;
		result["reward_soldier_type"] = 0;

		auto soldier_index = 0;

		for (auto i = 0u; i < player_data->get_staff_count(); i++)
		{
			const auto staff = player_data->get_staff(i);
			if (!database::player_data::is_usable_staff(staff) || 
				staff.status_sync.designation != database::player_data::des_security)
			{
				continue;
			}

			result["security_soldier"][soldier_index]["header"] = staff.packed_header;
			result["security_soldier"][soldier_index]["seed"] = staff.packed_seed;
			result["security_soldier"][soldier_index]["status_no_sync"] = staff.packed_status_no_sync;
			result["security_soldier"][soldier_index]["status_sync"] = staff.packed_status_sync;
			++soldier_index;
		}

		result["security_soldier_num"] = soldier_index;
		result["security_soldier_rank"] = 0;

		auto& stage_param = result["stage_param"];

		stage_param["build"] = {16385, 16385, 16385, 16385, 16385, 16385, 16385};
		stage_param["cluster_param"] = cluster_param[platform];
		stage_param["cluster_param"]["build"] = 0;

		stage_param["construct_param"] = fob->get_construct_param();

		stage_param["fob_index"] = fob_index;
		stage_param["mother_base_id"] = fob->get_id();
		stage_param["nuclear"] = player_data->get_nuke_count();
		stage_param["owner_player_id"] = player->get_id();

		static std::vector<std::string> resource_names =
		{
			"emplacement_gun_east",
			"emplacement_gun_west" ,
			"gatling_gun",
			"gatling_gun_east",
			"gatling_gun_west",
			"mortar_normal"
		};

		stage_param["placement"][resource_names[0]] = player_data->get_resource_value(database::player_data::processed_server, 34);
		stage_param["placement"][resource_names[1]] = player_data->get_resource_value(database::player_data::processed_server, 35);
		stage_param["placement"][resource_names[2]] = 0;
		stage_param["placement"][resource_names[3]] = player_data->get_resource_value(database::player_data::processed_server, 36);
		stage_param["placement"][resource_names[4]] = player_data->get_resource_value(database::player_data::processed_server, 37);
		stage_param["placement"][resource_names[5]] = player_data->get_resource_value(database::player_data::processed_server, 38);

		stage_param["platform"] = platform_j;
		stage_param["equip_grade"] = mother_base["equip_grade"];
		stage_param["security_level"] = mother_base["security_level"];

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
			stage_param["processing_resource"][material_resource_names[i]] =
				player_data->get_resource_value(database::player_data::unprocessed_server, i);
		}

		for (auto i = 0; i < database::player_data::unit_count; i++)
		{
			stage_param["section_level"][database::player_data::unit_names[i]] = player_data->get_unit_level(i);
		}

		for (auto i = 0; i < 5; i++)
		{
			stage_param["usable_resource"][material_resource_names[i]] =
				player_data->get_resource_value(database::player_data::processed_server, i);
		}

		result["wormhole_player_id"] = wormhole_player_id_j;

		if (!is_sneak)
		{
			const auto active_sneak = database::players::get_active_sneak(mother_base_id);
			if (!active_sneak.has_value())
			{
				result["result"] = "ERR_DATABASE";
				return result;
			}

			stage_param["platform"] = active_sneak->get_platform();

			if (!database::players::set_active_sneak(player->get_id(), fob->get_id(), fob->get_player_id(), active_sneak->get_platform(), mode,
				database::players::status_active, is_sneak))
			{
				result["result"] = "ERR_DATABASE";
				return result;
			}
		}
		else
		{
			if (!database::players::set_active_sneak(player->get_id(), fob->get_id(), fob->get_player_id(), platform, mode,
				database::players::status_active, is_sneak))
			{
				result["result"] = "ERR_DATABASE";
				return result;
			}
		}

		return result;
	}
}
