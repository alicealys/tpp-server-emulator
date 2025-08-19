#include <std_include.hpp>

#include "cmd_create_player.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"
#include "database/models/player_data.hpp"
#include "database/models/event_rankings.hpp"
#include "database/models/mgo_data.hpp"
#include "database/models/mgo_characters.hpp"
#include "database/models/fobs.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_create_player::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["xuid"] = {};

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		database::player_records::find_or_create(player->get_id());
		database::player_data::find_or_create(player->get_id());
		database::event_rankings::create_entries(player->get_id());

		if (database::vars.create_fobs)
		{
			const auto fobs = database::fobs::get_fob_list(player->get_id());
			if (fobs.empty())
			{
				database::fobs::create(player->get_id(), 0);
				database::fobs::create(player->get_id(), 0);
				database::fobs::create(player->get_id(), 0);
				database::fobs::create(player->get_id(), 0);
			}
		}

		const auto mgo_data = database::mgo_data::find(player->get_id());
		if (!mgo_data.has_value())
		{
			database::mgo_data::create(player->get_id());
			for (auto i = 0ull; i < database::mgo_characters::initial_character_count; i++)
			{
				database::mgo_characters::create_character(player->get_id(), static_cast<std::uint32_t>(i));
			}
		}

		result["player_id"] = player->get_id();
		result["result"] = "NOERR";

		return result;
	}
}
