#include <std_include.hpp>

#include "cmd_deploy_fob_assist.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_deploy_fob_assist::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& use_mb_coin_j = data["use_mb_coin"];
		const auto& mb_coin_pay_j = data["mb_coin_pay"];
		auto& deploy_exec_info = data["deploy_exec_info"];
		if (!mb_coin_pay_j.is_number_unsigned() || !use_mb_coin_j.is_number_unsigned() || !deploy_exec_info.is_object())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto& win_rate_j = deploy_exec_info["win_rate"];
		const auto& mother_base_id_j = deploy_exec_info["mother_base_id"];
		if (!win_rate_j.is_number_unsigned() || !mother_base_id_j.is_number_unsigned())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto mother_base_id = mother_base_id_j.get<std::uint32_t>();
		const auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		const auto win_rate = win_rate_j.get<std::uint32_t>();
		const auto rand = utils::cryptography::random::get_integer(0, 100);
		const auto win = rand <= win_rate;

		result["is_win"] = static_cast<std::uint32_t>(win);

		result["armored_east"] = 0;
		result["armored_east_rocket"] = 0;
		result["armored_west"] = 0;
		result["armored_west_wheeled"] = 0;
		result["car_east"] = 0;
		result["car_west"] = 0;
		result["tank_east"] = 0;
		result["tank_west"] = 0;
		result["truck_east"] = 0;
		result["truck_west"] = 0;
		result["walker_gear_cfa"] = 0;
		result["walker_gear_cfa_suppot"] = 0;
		result["walker_gear_proto"] = 0;
		result["walker_gear_soviet_support"] = 0;

		result["rewards"] = nlohmann::json::array();

		result["fob_deploy_damage_param"]["damage_values"] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		/*
		0 0: unk
		0 1: num guards
		0 2: num grade
		4 3: num laser detectors
		8 4: num anti theft device
		6 5: num cameras
		8 6: num claymores
		8 7: num guards 2
		0 8: anti-reflex
		0 9: reinforcements
		2 10: num drones
		*/
		if (!win)
		{
			result["fob_deploy_damage_param"]["cluster_index"] = 0;
			result["fob_deploy_damage_param"]["expiration_date"] = 0;
			result["fob_deploy_damage_param"]["motherbase_id"] = 0;

			return result;
		}
		else
		{
			const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
			const auto expire = now + 86400s;
			result["fob_deploy_damage_param"]["cluster_index"] = deploy_exec_info["cluster_index"];
			
			const auto& damage_values = deploy_exec_info["damage_values"];
			if (damage_values.is_array())
			{
				const auto count = std::min(static_cast<size_t>(database::player_data::damage_param_count), damage_values.size());
				for (auto i = 0; i < count; i++)
				{
					if (!damage_values[i].is_number_unsigned())
					{
						continue;
					}

					const auto value = damage_values[i].get<std::uint32_t>();
					const auto value_capped = std::min(value, database::player_data::deploy_damage_param_caps[i]);

					if (i == 9)
					{
						result["fob_deploy_damage_param"]["damage_values"][i] = value_capped * 100;
					}
					else
					{
						result["fob_deploy_damage_param"]["damage_values"][i] = value_capped;
					}
				}
			}

			result["fob_deploy_damage_param"]["expiration_date"] = expire.count();
			result["fob_deploy_damage_param"]["motherbase_id"] = deploy_exec_info["mother_base_id"];

			database::player_data::set_fob_deploy_damage_param(player->get_id(), result["fob_deploy_damage_param"]);
		}

		return result;
	}
}
