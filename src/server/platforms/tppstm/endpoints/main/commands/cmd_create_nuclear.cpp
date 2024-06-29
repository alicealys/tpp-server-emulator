#include <std_include.hpp>

#include "cmd_create_nuclear.hpp"

#include "database/models/items.hpp"
#include "database/models/player_data.hpp"

namespace tpp
{
	nlohmann::json create_nuclear(const std::optional<database::players::player>& player)
	{
		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.get())
		{
			return error(ERR_INVALIDARG);
		}

		const auto item = database::items::get_item(player->get_id(), database::items::nuclear);
		if (item.get_develop() != database::items::developed)
		{
			return error(ERR_INVALIDARG);
		}

		if (!database::items::remove(player->get_id(), database::items::nuclear))
		{
			return error(ERR_DATABASE);
		}

		database::player_data::resource_arrays_t resources{};
		player_data->copy_resources(resources);

		const auto inc = resources[database::player_data::processed_server][database::player_data::nuclear] + 1;
		const auto capped = database::player_data::cap_resource_value(database::player_data::processed_server, database::player_data::nuclear, inc);
		resources[database::player_data::processed_server][database::player_data::nuclear] = capped;

		database::player_data::set_resources(player->get_id(), resources, player_data->get_local_gmp(), player_data->get_server_gmp());

		return error(NOERR);
	}

	nlohmann::json cmd_create_nuclear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return create_nuclear(player);
	}

	bool cmd_create_nuclear::needs_player()
	{
		return true;
	}
}
