#include <std_include.hpp>

#include "cmd_get_fob_target_detail.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/wormholes.hpp"
#include "database/models/sneak_results.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_target_detail::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}
		
		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		auto& is_sneak_j = data["is_sneak"];
		auto& is_event_j = data["is_event"];
		auto& is_plus_j = data["is_plus"];
		auto& mode_j = data["mode"];
		auto& mother_base_id_j = data["mother_base_id"];

		if (!is_sneak_j.is_number_integer() || !is_event_j.is_number_integer() || !is_plus_j.is_number_integer() ||
			!mode_j.is_string() || !mother_base_id_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_event = is_event_j.get<std::uint32_t>() == 1;

		const auto mother_base_id = mother_base_id_j.get<std::uint64_t>();
		auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			return error(ERR_MOTHER_BASE_NOTFOUND);
		}

		const auto owner_data = database::player_data::find(fob->get_player_id());
		if (!owner_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto mode_str = mode_j.get<std::string>();
		const auto mode = database::players::get_sneak_mode_id(mode_str);
		if (mode == database::players::mode_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		auto& detail_j = result["detail"];

		detail_j["captured_rank_bottom"] = 0;
		detail_j["captured_rank_top"] = 0;
		detail_j["captured_staff"] = 0;
		detail_j["captured_rank_bottom"] = 0;

		auto& mother_base_param_j = detail_j["mother_base_param"];
		mother_base_param_j["area_id"] = 0;
		mother_base_param_j["fob_index"] = fob->get_index();
		mother_base_param_j["platform_count"] = 0;
		mother_base_param_j["price"] = 0;
		mother_base_param_j["security_rank"] = 0;
		mother_base_param_j["cluster_param"] = nlohmann::json::array();

		auto& cluster_param = fob->get_cluster_param();

		const auto current_deploy = database::fob_deploy_assists::get_current_deploy(player->get_id());
		if (!database::players::is_system_player_id(fob->get_player_id()) && 
			current_deploy.has_value() && fob->get_id() == current_deploy->get_fob_id())
		{
			current_deploy->apply(cluster_param);
		}

		for (auto i = 0u; i < game::fob_clusters_count; i++)
		{
			const auto& param = cluster_param.param[i];
			param.to_json(mother_base_param_j["cluster_param"][i]);
		}

		mother_base_param_j["construct_param"] = fob->get_construct_param().packed;
		mother_base_param_j["mother_base_id"] = fob->get_id();

		detail_j["owner_player_id"] = fob->get_player_id();

		const auto owner = database::players::find(fob->get_player_id());
		if (!owner.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto owner_record = database::player_records::find(fob->get_player_id());
		if (!owner_record.has_value())
		{
			return error(ERR_DATABASE);
		}

		database::player_data::resource_arrays_t resource_arrays{};
		owner_data->get_resource_arrays(resource_arrays);

		game::fob_placements_t placements{};
		placements.emplacement_gun_east = resource_arrays[game::processed_server][game::EMPLACEMENT_GUN_EAST];
		placements.emplacement_gun_west = resource_arrays[game::processed_server][game::EMPLACEMENT_GUN_WEST];
		placements.gatling_gun_east = resource_arrays[game::processed_server][game::ANTI_AIR_GATLING_GUN_EAST];
		placements.gatling_gun_west = resource_arrays[game::processed_server][game::ANTI_AIR_GATLING_GUN_WEST];
		placements.mortar_normal = resource_arrays[game::processed_server][game::MORTAR_NORMAL];
		placements.to_json(detail_j["placement"]);

		database::sneak_results::sneak_reward_previews_t primary_reward{};
		database::sneak_results::generate_sneak_reward_previews(cluster_param, primary_reward);
		primary_reward.to_json(detail_j["primary_reward"]);

		const auto event_player = database::fob_events::get_player(owner->get_id());
		if (is_event && event_player.has_value())
		{
			detail_j["reward_rate"] = 200;
			detail_j["platform"] = 255;
		}
		else
		{
			detail_j["reward_rate"] = 0;
			detail_j["platform"] = 0;
		}

		database::player_data::staff_array_container staff_array;
		owner_data->get_staff_array(staff_array);
		std::array<std::array<std::uint16_t, 7>, 10> section_staff{};

		for (auto i = 0u; i < owner_data->get_staff_count(); i++)
		{
			const auto& staff = staff_array[i];
			if (!game::is_usable_staff(staff))
			{
				continue;
			}

			const auto peak_rank = game::calc_staff_top_rank(staff);
			switch (staff.fields.status_sync.designation)
			{
			case game::SECTION_COMBAT:
				++section_staff[peak_rank][0];
				break;
			case game::SECTION_DEVELOP:
				++section_staff[peak_rank][1];
				break;
			case game::SECTION_BASE_DEV:
				++section_staff[peak_rank][2];
				break;
			case game::SECTION_SUPPORT:
				++section_staff[peak_rank][3];
				break;
			case game::SECTION_SPY:
				++section_staff[peak_rank][4];
				break;
			case game::SECTION_MEDICAL:
				++section_staff[peak_rank][5];
				break;
			case game::SECTION_SECURITY:
				++section_staff[peak_rank][6];
				break;
			}
		}

		auto& section_staff_j = detail_j["section_staff"];
		for (auto i = 0ull; i < section_staff.size(); i++)
		{
			auto& staff = section_staff_j[i];
			staff["combat"] = section_staff[i][0];
			staff["develop"] = section_staff[i][1];
			staff["base"] = section_staff[i][2];
			staff["suport"] = section_staff[i][3];
			staff["medical"] = section_staff[i][4];
			staff["spy"] = section_staff[i][5];
			staff["security"] = section_staff[i][6];
		}

		detail_j["security_section_rank"] = owner_data->unit_levels.value[game::unit_security];

		const auto is_actual = mode == database::players::mode_actual;

		result["event_clear_bit"] = 0;
		result["is_restrict"] = is_actual ? stats->is_shield_active() : 0;

		auto& session_j = result["session"];
		session_j["ip"] = "0.0.0.0";
		session_j["is_invalid"] = 1;
		session_j["npid"]["handler"]["data"] = "";
		session_j["npid"]["handler"]["dummy"] = {0, 0, 0};
		session_j["npid"]["handler"]["term"] = 0;
		session_j["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
		session_j["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
		session_j["port"] = 0;
		session_j["secure_device_address"] = "NotImplement";
		session_j["steamid"] = 0;
		session_j["xnaddr"] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
		session_j["xnkey"] = {};
		session_j["xnkid"] = {};
		session_j["xuid"] = 0;

		if (!owner->is_real_player() && is_actual)
		{
			const auto wormhole = database::wormholes::get_wormhole_status(player->get_id(), owner->get_id());
			if ((!wormhole.is_open() || !wormhole.first) && owner_record->is_shield_active())
			{
				return error(ERR_SNEAK_RESTRICTION);
			}
		}

		const auto is_sneak = is_sneak_j.get<std::uint32_t>() == 1;
		if (is_sneak)
		{
			session_j["ip"] = player->get_ex_ip();
			session_j["is_invalid"] = 0;
			session_j["port"] = player->get_ex_port();
			session_j["steamid"] = player->get_account_id();
			session_j["xuid"] = player->get_account_id();

			if (!database::players::set_active_sneak(player->get_id(), fob->get_id(), fob->get_player_id(), 0u, 0u, mode, 
				database::players::status_menu, true, owner->is_security_challenge_enabled(), owner->is_real_player()))
			{
				return error(ERR_ALREADY_SNEAK);
			}
		}
		else
		{
			if (!owner->is_real_player())
			{
				return error(ERR_DATABASE);
			}

			const auto active_sneak = database::players::find_active_sneak(owner->get_id(), mode);
			if (!active_sneak.has_value() || active_sneak->get_fob_id() != fob->get_id())
			{
				return error(ERR_DATABASE);
			}

			const auto attacker = database::players::find(active_sneak->get_player_id());
			if (!attacker.has_value())
			{
				return error(ERR_DATABASE);
			}

			detail_j["platform"] = active_sneak->get_platform();

			session_j["ip"] = attacker->get_ex_ip();
			session_j["is_invalid"] = 0;
			session_j["port"] = attacker->get_ex_port();
			session_j["steamid"] = attacker->get_account_id();
			session_j["xuid"] = attacker->get_account_id();

			if (!database::players::set_active_sneak(player->get_id(), fob->get_id(), fob->get_player_id(), 0u, 
				active_sneak->get_platform(), mode, database::players::status_menu, false, 
				owner->is_security_challenge_enabled()))
			{
				return error(ERR_ALREADY_SNEAK);
			}
		}

		return result;
	}

	bool cmd_get_fob_target_detail::needs_player()
	{
		return true;
	}
}
