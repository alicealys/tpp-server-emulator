#include <std_include.hpp>

#include "cmd_create_nuclear.hpp"

#include "database/models/items.hpp"
#include "database/models/player_data.hpp"
#include "database/models/variables.hpp"

namespace emulator::tpp
{
	cmd_create_nuclear::cmd_create_nuclear()
	{
		database::variables::register_lock("abolition_lock");
	}

	nlohmann::json create_nuclear(const std::optional<database::players::player>& player)
	{
		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
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
		player_data->get_resource_arrays(resources);

		const auto inc = resources[game::processed_server][game::NUCLEAR_WEAPON] + 1;
		const auto capped = game::cap_resource_value(game::processed_server, game::NUCLEAR_WEAPON, inc);
		resources[game::processed_server][game::NUCLEAR_WEAPON] = capped;

		database::player_data::set_resources(player->get_id(), resources, player_data->get_local_gmp(), player_data->get_server_gmp());

		const auto nuke_count = database::player_data::get_nuke_count();
		database::variables::access_with_lock("abolition_lock", [&]()
		{
			const auto max_nuke_count = database::variables::get<std::uint32_t>("abolition_max", 0);
			if (nuke_count > max_nuke_count)
			{
				database::variables::set("abolition_max", nuke_count);
			}
		});

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
