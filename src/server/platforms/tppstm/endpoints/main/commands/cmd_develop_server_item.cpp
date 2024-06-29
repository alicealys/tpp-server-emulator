#include <std_include.hpp>

#include "cmd_develop_server_item.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_develop_server_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto p_data = database::player_data::find(player->get_id());
		if (!p_data.get())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& item_id_j = data["id"];
		if (!item_id_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		const auto item_id = item_id_j.get<std::uint32_t>();
		auto item_list = database::items::get_item_list(player->get_id());
		auto dev_count = 0u;
		for (const auto& status : item_list)
		{
			if (status.second.get_develop() == 1)
			{
				++dev_count;
			}

			if (status.second.get_id() == item_id)
			{
				if (status.second.is_created())
				{
					return error(ERR_SERVERITEM_ALREADY_DEVELOPED);
				}

				if (!status.second.is_open())
				{
					return error(ERR_SERVERITEM_UNOPENFLAG);
				}
			}
		}

		if (dev_count >= database::vars.item_dev_limit)
		{
			return error("ERR_SERVERITEM_DEVLIMIT"); // made up error
		}

		database::player_data::resource_arrays_t resource_arrays{};
		p_data->copy_resources(resource_arrays);
		const auto& item = item_list[item_id];

		resource_arrays[database::player_data::processed_server][item.get_resource_id(0)] -= item.get_resource_value(0);
		resource_arrays[database::player_data::processed_server][item.get_resource_id(1)] -= item.get_resource_value(1);
		const auto server_gmp = p_data->get_server_gmp() - item.get_gmp();

		database::items::create(player->get_id(), item_id);
		database::player_data::set_resources(player->get_id(), resource_arrays, p_data->get_local_gmp(), server_gmp);

		result["result"] = utils::tpp::get_error(NOERR);
		result["xuid"] = {};

		return result;
	}
}
