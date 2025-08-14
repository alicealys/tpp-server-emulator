#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_set_mgo_match_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_match_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& abandon_j = data["abandon"];
		const auto& started_j = data["started"];
		const auto& played_j = data["played"];

		if (!abandon_j.is_number_unsigned() || !started_j.is_number_unsigned() || !played_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto abandon = abandon_j.get<std::uint32_t>();
		const auto started = started_j.get<std::uint32_t>();
		const auto played = played_j.get<std::uint32_t>();

		database::mgo_data::update_match_stats(player->get_id(), abandon, started, played);

		return result;
	}

	bool cmd_set_mgo_match_stat::needs_player()
	{
		return true;
	}
}
