#include <std_include.hpp>

#include "cmd_send_deploy_injure.hpp"

#include "database/models/combat_deployments.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	namespace
	{
		bool handle_dead_soldier(database::player_data::player_data& player_data, nlohmann::json& dead_soldier)
		{
			if (!dead_soldier.is_array())
			{
				return false;
			}

			database::player_data::staff_array_container staff_array;
			player_data.get_staff_array(staff_array);

			std::vector<game::staff_t> to_delete;

			for (auto i = 0u; i < dead_soldier.size(); i++)
			{
				auto& soldier_j = dead_soldier[i];
				if (!soldier_j.is_object())
				{
					continue;
				}

				auto& param = soldier_j["param"];
				if (!param.is_array() || param.size() != 4)
				{
					continue;
				}

				game::staff_t staff{};
				staff.fields.packed_header = param[0].get<std::uint32_t>();
				staff.fields.packed_seed = param[1].get<std::uint32_t>();
				staff.fields.packed_status_sync = param[2].get<std::uint32_t>();
				staff.fields.packed_status_no_sync = param[3].get<std::uint16_t>();
			}

			const auto should_delete = [&](game::staff_t& staff)
			{
				for (auto& del : to_delete)
				{
					if (del.fields.packed_header == staff.fields.packed_header && del.fields.packed_header == staff.fields.packed_seed)
					{
						return true;
					}
				}

				return false;
			};

			database::player_data::staff_array_container new_staff_array;

			auto idx = 0;
			for (auto i = 0u; i < game::max_staff_count; i++)
			{
				if ((staff_array[i].fields.packed_header == 0 && staff_array[i].fields.packed_seed == 0) || should_delete(staff_array[i]))
				{
					continue;
				}

				new_staff_array[idx++] = staff_array[i];
			}

			database::player_data::set_soldier_bin(player_data.get_player_id(), idx, new_staff_array);

			return true;
		}
	}

	nlohmann::json cmd_send_deploy_injure::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& injure_list_j = data["injure_list"];

		if (!injure_list_j.is_array())
		{
			return error(ERR_INVALIDARG);
		}

		auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		static std::unordered_map<std::string, std::uint32_t> resource_type_map =
		{
			{"armored_east", game::ARMORED_VEHICLE_EAST},
			{"armored_east_rocket", game::ARMORED_VEHICLE_EAST_ROCKET},
			{"armored_west", game::ARMORED_VEHICLE_WEST},
			{"armored_west_wheeled", game::WHEELED_ARMORED_VEHICLE_WEST},
			{"car_east", game::CAR_EAST},
			{"car_west", game::CAR_WEST},
			{"tank_east", game::TANK_EAST},
			{"tank_west", game::TANK_WEST},
			{"truck_east", game::TRUCK_EAST},
			{"truck_west", game::TRUCK_WEST},
			{"walker_gear_cfa", game::WALKER_GEAR_CFA_BATTLE},
			{"walker_gear_cfa_support", game::WALKER_GEAR_CFA_SUPPORT},
			{"walker_gear_proto", game::WALKER_GEAR_PROTO_HEUY},
			{"walker_gear_soviet", game::WALKER_GEAR_SOVIET_BATTLE},
			{"walker_gear_soviet_support", game::WALKER_GEAR_SOVIET_SUPPORT},
		};

		database::player_data::resource_arrays_t new_resources{};
		player_data->get_resource_arrays(new_resources);

		for (auto i = 0u; i < injure_list_j.size(); i++)
		{
			if (!injure_list_j[i].is_object())
			{
				continue;
			}

			auto& id_j = injure_list_j[i]["id"];
			if (!id_j.is_number_unsigned())
			{
				continue;
			}

			const auto mission_id = id_j.get<std::uint32_t>();
			const auto deployment = database::combat_deployments::get_deployment(player->get_id(), mission_id);
			if (!deployment.has_value() || deployment->completed())
			{
				continue;
			}

			handle_dead_soldier(player_data.value(), injure_list_j[i]["dead_soldier"]);

			for (auto& [name, resource_type] : resource_type_map)
			{
				if (!injure_list_j[i][name].is_number_unsigned())
				{
					continue;
				}

				const auto value = injure_list_j[i][name].get<std::uint32_t>();
				const auto sub = std::min(value, new_resources[game::processed_server][resource_type]);
				new_resources[game::processed_server][resource_type] -= sub;
			}
		}

		database::player_data::set_resources(player->get_id(), new_resources, player_data->get_local_gmp(), player_data->get_server_gmp());

		return result;
	}

	bool cmd_send_deploy_injure::needs_player()
	{
		return true;
	}
}
