#include <std_include.hpp>

#include "cmd_add_follow.hpp"

#include "database/models/players.hpp"
#include "database/models/player_follows.hpp"

namespace tpp
{
	nlohmann::json cmd_add_follow::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& steam_id_j = data["steam_id"];
		const auto& player_id_j = data["player_id"];

		if (!steam_id_j.is_number_unsigned() && !player_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto to_steam_id = steam_id_j.get<std::uint64_t>();
		auto to_player_id = player_id_j.get<std::uint64_t>();

		if (to_steam_id != 0)
		{
			const auto to_player = database::players::find_from_account(to_steam_id);
			if (!to_player.has_value())
			{
				return error(ERR_PLAYER_NOTFOUND);
			}

			to_player_id = to_player->get_id();
		}
		else
		{
			to_player_id = player_id_j.get<std::uint64_t>();
			if (!database::players::exists(to_player_id))
			{
				return error(ERR_PLAYER_NOTFOUND);
			}
		}

		if (to_player_id == player->get_id())
		{
			return error(ERR_ALREADY_BELONG);
		}

		const auto follows = database::player_follows::get_follows(player->get_id());
		if (follows.size() >= database::player_follows::max_follows)
		{
			return error(ERR_OVER_CAPACITY);
		}

		if (follows.contains(to_player_id))
		{
			return error(ERR_ALREADY_BELONG);
		}

		if (!database::player_follows::add_follow(player->get_id(), to_player_id))
		{
			return error(ERR_DATABASE);
		}

		return error(NOERR);
	}

	bool cmd_add_follow::needs_player()
	{
		return true;
	}
}
