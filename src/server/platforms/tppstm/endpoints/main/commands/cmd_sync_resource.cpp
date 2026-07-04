#include <std_include.hpp>

#include "cmd_sync_resource.hpp"
#include "cmd_create_nuclear.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_sync_resource::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		create_nuclear(player);

		const auto& version_j = data["version"];
		const auto& diff_resource_1 = data["diff_resource1"];
		const auto& diff_resource_2 = data["diff_resource2"];
		const auto& gmp_j = data["gmp"];

		if (!gmp_j.is_number_integer() ||
			!diff_resource_1.is_array() || !diff_resource_2.is_array() ||
			diff_resource_1.size() < game::RESOURCE_TYPE_COUNT ||
			diff_resource_2.size() < game::RESOURCE_TYPE_COUNT)
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		database::player_data::resource_arrays_t resource_arrays{};
		player_data->get_resource_arrays(resource_arrays);

		const auto sync_resources = [&](const nlohmann::json& resources, 
			const game::resource_array_types_t local_type,
			const game::resource_array_types_t server_type,
			bool sync)
		{
			const auto id = local_type == game::unprocessed_local ? "2"s : "1"s;
			for (auto i = 0; i < static_cast<std::int32_t>(resources.size()); i++)
			{
				if (sync)
				{
					const auto& value_j = resources[i];
					if (!value_j.is_number_integer())
					{
						return false;
					}

					const auto ratio = game::get_local_resource_ratio(local_type, server_type, i);

					const auto current_local_value = game::cap_resource_value(local_type, i, value_j.get<std::uint32_t>());
					const auto current_server_value = i == game::NUCLEAR_WEAPON ? 
						player_data->get_nuke_count() : 
						game::cap_resource_value(server_type, i, resource_arrays[server_type][i]);

					const auto total = current_local_value + current_server_value;

					const auto local_value = game::cap_resource_value(local_type, i, static_cast<std::uint32_t>(total * ratio));
					const auto server_value = game::cap_resource_value(server_type, i, total - local_value);

					resource_arrays[local_type][i] = local_value;
					resource_arrays[server_type][i] = server_value;
				}

				result["diff_resource" + id][i] = resource_arrays[local_type][i];
				result["fix_resource" + id][i] = resource_arrays[server_type][i];
			}

			return true;
		};

		auto server_gmp = player_data->get_server_gmp();
		auto local_gmp = std::min(database::vars.max_local_gmp, gmp_j.get<std::int32_t>());

		const auto local_version = version_j.get<std::uint64_t>();
		const auto client_version = player_data->get_client_resource_version();
		auto server_version = player_data->get_server_resource_version();

		const auto update_server = client_version == server_version && local_version != client_version - 1;

		if (!sync_resources(diff_resource_1, game::processed_local, game::processed_server, update_server) ||
			!sync_resources(diff_resource_2, game::unprocessed_local, game::unprocessed_server, update_server))
		{
			return error(ERR_INVALIDARG);
		}

		if (update_server)
		{
			const auto total_gmp = local_gmp + server_gmp;
			local_gmp = std::min(database::vars.max_local_gmp, static_cast<std::int32_t>(database::vars.gmp_ratio * total_gmp));
			server_gmp = std::min(database::vars.max_server_gmp, total_gmp - local_gmp);

			database::player_data::set_resources_as_sync(player->get_id(), resource_arrays, local_gmp, server_gmp);
			server_version++;
		}

		database::player_data::sync_client_resource_version(player->get_id());

		result["version"] = server_version;

		result["injury_gmp"] = 0;
		result["insurance_gmp"] = 0;
		result["loadout_gmp"] = 0;
		result["diff_gmp"] = 0;

		result["local_gmp"] = local_gmp;
		result["server_gmp"] = server_gmp;

		return result;
	}
}
