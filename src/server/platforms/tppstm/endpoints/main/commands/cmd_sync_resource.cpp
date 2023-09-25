#include <std_include.hpp>

#include "cmd_sync_resource.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_resource::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

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
			const database::player_data::resource_array_types server_type,
			bool sync)
		{
			const auto id = local_type == database::player_data::unprocessed_local ? "2"s : "1"s;
			for (auto i = 0; i < resources.size(); i++)
			{
				const auto& value_j = resources[i];
				if (!value_j.is_number_integer())
				{
					return;
				}

				const auto ratio = database::player_data::get_local_resource_ratio(local_type, server_type, i);

				const auto current_local_value = database::player_data::cap_resource_value(local_type, i, value_j.get<std::uint32_t>());
				const auto current_server_value = database::player_data::cap_resource_value(server_type, i, resource_arrays[server_type][i]);

				const auto total = current_local_value + current_server_value;

				const auto local_value = database::player_data::cap_resource_value(local_type, i, static_cast<std::uint32_t>(total * ratio));
				const auto server_value = database::player_data::cap_resource_value(server_type, i, total - local_value);

				resource_arrays[local_type][i] = local_value;
				resource_arrays[server_type][i] = server_value;

				result["diff_resource" + id][i] = resource_arrays[local_type][i];
				result["fix_resource" + id][i] = resource_arrays[server_type][i];
			}
		};

		auto server_gmp = player_data->get_server_gmp();
		auto local_gmp = std::min(database::player_data::max_local_gmp, gmp_j.get<std::int32_t>());

		const auto now = std::chrono::system_clock::now();
		const auto now_epoc = now.time_since_epoch();
		const auto diff = now_epoc - player_data->get_last_sync();
		const auto should_sync = diff >= 30min;

		sync_resources(diff_resource_1, database::player_data::processed_local, database::player_data::processed_server, should_sync);
		sync_resources(diff_resource_2, database::player_data::unprocessed_local, database::player_data::unprocessed_server, should_sync);

		if (should_sync)
		{
			const auto total_gmp = local_gmp + server_gmp;
			local_gmp = std::min(database::player_data::max_local_gmp, static_cast<std::int32_t>(database::player_data::gmp_ratio * total_gmp));
			server_gmp = std::min(database::player_data::max_server_gmp, total_gmp - local_gmp);

			database::player_data::set_resources_as_sync(player->get_id(), resource_arrays, local_gmp, server_gmp);
			result["version"] = player_data->get_version() + 1;
		}
		else
		{
			result["version"] = player_data->get_version();
		}

		result["injury_gmp"] = 0;
		result["insurance_gmp"] = 0;
		result["loadout_gmp"] = 0;
		result["diff_gmp"] = 0;

		result["local_gmp"] = local_gmp;
		result["server_gmp"] = server_gmp;

		return result;
	}
}
