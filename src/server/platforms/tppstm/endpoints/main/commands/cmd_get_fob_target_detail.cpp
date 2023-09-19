#include <std_include.hpp>

#include "cmd_get_fob_target_detail.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_target_detail::execute(nlohmann::json& data, const std::string& session_key)
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

		if (!is_sneak_j.is_number_integer() || !is_event_j.is_number_integer() || !is_plus_j.is_number_integer()
			|| !mode_j.is_string() || !mother_base_id_j.is_number_integer())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto player_data = database::player_data::find(fob->get_player_id());
		auto& detail = result["detail"];

		detail["captured_rank_bottom"] = 0;
		detail["captured_rank_top"] = 0;
		detail["captured_staff"] = 0;
		detail["captured_rank_bottom"] = 0;

		detail["mother_base_param"]["area_id"] = 0;
		detail["mother_base_param"]["fob_index"] = 0;
		detail["mother_base_param"]["platform_count"] = 0;
		detail["mother_base_param"]["price"] = 0;
		detail["mother_base_param"]["security_rank"] = 0;
		detail["mother_base_param"]["cluster_param"] = fob->get_cluster_param();
		detail["mother_base_param"]["construct_param"] = fob->get_construct_param();
		detail["mother_base_param"]["mother_base_id"] = fob->get_id();

		detail["owner_player_id"] = fob->get_player_id();

		static std::vector<std::string> resource_names =
		{
			"emplacement_gun_east",
			"emplacement_gun_west" ,
			"gatling_gun",
			"gatling_gun_east",
			"gatling_gun_west",
			"mortar_normal"
		};

		detail["placement"][resource_names[0]] = player_data->get_resource_value(database::player_data::processed_server, 34);
		detail["placement"][resource_names[1]] = player_data->get_resource_value(database::player_data::processed_server, 35);
		detail["placement"][resource_names[2]] = 0;
		detail["placement"][resource_names[3]] = player_data->get_resource_value(database::player_data::processed_server, 36);
		detail["placement"][resource_names[4]] = player_data->get_resource_value(database::player_data::processed_server, 37);
		detail["placement"][resource_names[5]] = player_data->get_resource_value(database::player_data::processed_server, 38);

		detail["platform"] = 255;
		detail["reward_rate"] = 0;

		detail["primary_reward"] = nlohmann::json::array();

		for (auto i = 0u; i < 10; i++)
		{
			for (const auto& name : database::player_data::unit_names)
			{
				detail["section_staff"][i][name] = 0;
			}
		}

		for (auto i = 0u; i < player_data->get_staff_count(); i++)
		{
			const auto staff = player_data->get_staff(i);
			const auto key_opt = database::player_data::unit_name_from_designation(staff.status_sync.designation);
			if (!key_opt.has_value())
			{
				continue;
			}

			if (!database::player_data::is_usable_staff(staff))
			{
				continue;
			}

			const auto& key = key_opt.value();
			const auto& value = detail["section_staff"][staff.header.peak_rank][key];
			detail["section_staff"][staff.header.peak_rank][key] = 1 + value.get<std::uint32_t>();
		}

		detail["security_section_rank"] = player_data->get_unit_level(database::player_data::des_security);

		result["event_clear_bit"] = 0;
		result["is_restrict"] = 0;

		result["session"]["ip"] = player->get_ex_ip();
		result["session"]["is_invalid"] = 0;
		result["session"]["npid"]["handler"]["data"] = "";
		result["session"]["npid"]["handler"]["dummy"] = {0, 0, 0};
		result["session"]["npid"]["handler"]["term"] = 0;
		result["session"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
		result["session"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
		result["session"]["port"] = player->get_ex_port();
		result["session"]["secure_device_address"] = "NotImplement";
		result["session"]["steamid"] = player->get_account_id();
		result["session"]["xnaddr"] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
		result["session"]["xnkey"] = {};
		result["session"]["xnkid"] = {};
		result["session"]["xuid"] = player->get_account_id();

		return result;
	}
}
