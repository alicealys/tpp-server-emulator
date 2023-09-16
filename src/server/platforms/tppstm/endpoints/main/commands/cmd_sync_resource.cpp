#include <std_include.hpp>

#include "cmd_sync_resource.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_resource::execute(const nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& diff_resource_1 = data["diff_resource1"];
		const auto& diff_resource_2 = data["diff_resource2"];
		const auto& gmp_j = data["gmp"];

		if (!gmp_j.is_number_integer() ||
			!diff_resource_1.is_array() || !diff_resource_2.is_array() ||
			diff_resource_1.size() < database::player_data::resource_type_count ||
			diff_resource_2.size() < database::player_data::resource_type_count)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.get())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		database::player_data::resource_arrays_t resource_arrays{};
		player_data->copy_resources(resource_arrays);

		const auto sync_resources = [&](const nlohmann::json& resources, 
			const database::player_data::resource_array_types local_type, 
			const database::player_data::resource_array_types server_type)
		{
			const auto id = local_type == database::player_data::unprocessed_local ? "2"s : "1"s;
			for (auto i = 0; i < resources.size(); i++)
			{
				const auto& value_j = resources[i];
				if (!value_j.is_number_integer())
				{
					return;
				}

				const auto max = database::player_data::get_max_resource_value(local_type, i);
				const auto max_server = database::player_data::get_max_resource_value(server_type, i);
				resource_arrays[local_type][i] = database::player_data::cap_resource_value(local_type, i, value_j.get<std::uint32_t>());

				if (resource_arrays[server_type][i] < max_server)
				{
					const auto transfer_amount = std::min(max_server - resource_arrays[server_type][i],
						static_cast<std::uint32_t>(max * 0.1f));

					printf("transfering %i resource %i to server\n", transfer_amount, i);

					resource_arrays[local_type][i] -= transfer_amount;
					resource_arrays[server_type][i] += transfer_amount;
				}

				result["diff_resource" + id][i] = resource_arrays[local_type][i];
				result["fix_resource" + id][i] = resource_arrays[server_type][i];
			}
		};

		sync_resources(diff_resource_1, database::player_data::processed_local, database::player_data::processed_server);
		sync_resources(diff_resource_2, database::player_data::unprocessed_local, database::player_data::unprocessed_server);

		result["injury_gmp"] = 0;
		result["insurance_gmp"] = 0;
		result["loadout_gmp"] = 0;
		result["diff_gmp"] = 0;

		auto server_gmp = player_data->get_server_gmp();
		auto local_gmp = std::min(database::player_data::max_local_gmp, gmp_j.get<std::uint32_t>());

		printf("server_gmp = %i\n", server_gmp);
		printf("local_gmp = %i\n", local_gmp);

		if (server_gmp < database::player_data::max_server_gmp)
		{
			const auto transfer_amount = std::min(database::player_data::max_server_gmp - server_gmp,
				static_cast<std::uint32_t>(local_gmp * 0.8f));

			server_gmp += transfer_amount;
			local_gmp -= transfer_amount;

			printf("transfering %i gmp to server\n", transfer_amount);

			result["local_gmp"] = local_gmp;
			result["server_gmp"] = server_gmp;
		}

		database::player_data::set_resources(player->get_id(), resource_arrays, local_gmp, server_gmp);
		result["version"] = player_data->get_version() + 1;

		return result;
	}
}
