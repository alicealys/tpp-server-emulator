#include <std_include.hpp>

#include "database/models/player_data.hpp"

#include "cmd_destruct_online_nuclear.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_destruct_online_nuclear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& count_j = data["count"];
		if (!count_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto nuke_count = database::player_data::get_player_nuke_count(player->get_id());
		const auto destruct_count = std::min(nuke_count, count_j.get<std::uint32_t>());

		if (!database::player_data::set_nuke_count(player->get_id(), nuke_count - destruct_count))
		{
			return error(ERR_DATABASE);
		}

		return result;
	}

	bool cmd_destruct_online_nuclear::needs_player()
	{
		return true;
	}
}
