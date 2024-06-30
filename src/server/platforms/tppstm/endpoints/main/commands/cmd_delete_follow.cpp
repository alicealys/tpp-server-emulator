#include <std_include.hpp>

#include "cmd_delete_follow.hpp"

#include "database/models/players.hpp"
#include "database/models/player_follows.hpp"

namespace tpp
{
	nlohmann::json cmd_delete_follow::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& player_id_j = data["player_id"];

		if (!player_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto to_player_id = player_id_j.get<std::uint64_t>();
		if (!database::players::exists(to_player_id))
		{
			return error(ERR_PLAYER_NOTFOUND);
		}

		if (to_player_id == player->get_id())
		{
			return error(NOERR);
		}

		if (!database::player_follows::remove_follow(player->get_id(), to_player_id))
		{
			return error(ERR_DATABASE);
		}

		return error(NOERR);
	}

	bool cmd_delete_follow::needs_player()
	{
		return true;
	}
}
