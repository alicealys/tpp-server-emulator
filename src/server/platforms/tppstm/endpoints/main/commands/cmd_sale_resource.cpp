#include <std_include.hpp>

#include "cmd_sale_resource.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sale_resource::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto& num_j = data["num"];
		const auto& resource_id_j = data["resouce_id"]; // typo in the game
		const auto& unit_price_j = data["unit_price"];

		if (!num_j.is_number_integer() || !resource_id_j.is_number_integer() || !unit_price_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		const auto resource_id = resource_id_j.get<std::uint32_t>();
		if (resource_id > database::player_data::resource_type_count)
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.get())
		{
			return error(ERR_PLAYER_NOTFOUND);
		}

		const auto num = num_j.get<std::uint32_t>();
		const auto unit_price = unit_price_j.get<std::uint32_t>();

		database::player_data::resource_arrays_t resource_arrays{};
		player_data->copy_resources(resource_arrays);

		if (resource_arrays[database::player_data::processed_server][resource_id] < num)
		{
			return error(ERR_INVALIDARG);
		}

		resource_arrays[database::player_data::processed_server][resource_id] -= num;

		auto server_gmp = player_data->get_server_gmp();
		auto local_gmp = player_data->get_local_gmp();

		const auto total_profit = num * unit_price;
		server_gmp += total_profit;

		const auto left = (server_gmp - database::player_data::max_server_gmp);

		if (left > 0)
		{
			server_gmp -= left;
			local_gmp += left;

			const auto local_left = (local_gmp - database::player_data::max_local_gmp);
			if (local_left > 0)
			{
				local_gmp -= local_left;
			}
		}

		result["result_gmp"] = server_gmp;
		result["result_num"] = resource_arrays[database::player_data::processed_server][resource_id];
		result["resouce_id"] = resource_id;

		database::player_data::set_resources(player->get_id(), resource_arrays, local_gmp, server_gmp);
		result["version"] = player_data->get_version() + 1;

		return result;
	}
}
