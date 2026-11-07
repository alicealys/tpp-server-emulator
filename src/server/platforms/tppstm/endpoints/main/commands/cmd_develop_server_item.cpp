#include <std_include.hpp>

#include "cmd_develop_server_item.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_develop_server_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& item_id_j = data["id"];
		if (!item_id_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		database::player_data::resource_arrays_t resource_arrays{};
		database::player_data::staff_array_container staff_array{};

		player_data->get_resource_arrays(resource_arrays);
		player_data->get_staff_array(staff_array);

		const auto item_id = item_id_j.get<std::uint32_t>();
		const auto items = database::items::get_item_list(player->get_id());
		auto dev_count = 0u;

		const auto iter = items.find(item_id);
		if (iter == items.end() || !iter->is_open())
		{
			return error(ERR_SERVERITEM_UNOPENFLAG);
		}

		const auto& item = *iter;
		if (item.is_created())
		{
			return error(ERR_SERVERITEM_ALREADY_DEVELOPED);
		}

		if (!item.can_develop(player_data, resource_arrays, staff_array))
		{
			return error(ERR_SERVERITEM_RESOURCE_SHORTAGE);
		}

		for (const auto& status : items)
		{
			if (status.get_develop() == database::items::item_develop_indev && status.get_item_type() != database::items::nuclear)
			{
				++dev_count;
			}

			if (dev_count >= database::vars.item_dev_limit)
			{
				return error("ERR_SERVERITEM_DEVLIMIT"); // made up error
			}
		}

		const auto id_0 = item.get_resource_id(0);
		const auto id_1 = item.get_resource_id(1);

		const auto value_0 = item.get_resource_value(0);
		const auto value_1 = item.get_resource_value(1);

		const auto sub_resource = [&](std::uint32_t id, std::uint32_t amount)
		{
			if (id >= game::RESOURCE_TYPE_COUNT)
			{
				return;
			}

			auto& value = resource_arrays[game::processed_server][id];
			const auto cap_amount = std::min(amount, value);
			value -= cap_amount;
		};

		sub_resource(id_0, value_0);
		sub_resource(id_1, value_1);

		auto server_gmp = player_data->get_server_gmp();
		server_gmp -= std::min(server_gmp, static_cast<std::int32_t>(item.get_gmp()));

		if (item.get_item_develop_id() != 0)
		{
			database::items::create_existing(item.get_item_develop_id(), database::items::item_state_develop);
		}
		else
		{
			database::items::create(player->get_id(), item_id, database::items::item_state_develop);
		}

		database::player_data::set_resources_gmp(player->get_id(), resource_arrays, player_data->get_local_gmp(), server_gmp);

		return result;
	}
}
