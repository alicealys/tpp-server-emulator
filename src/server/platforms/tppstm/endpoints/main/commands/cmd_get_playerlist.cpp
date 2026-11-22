#include <std_include.hpp>

#include "cmd_get_playerlist.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_playerlist::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto stats = database::player_records::find(player->get_id());
		const auto player_data = database::player_data::find(player->get_id());


		if (stats.has_value() && player_data.has_value())
		{
			auto& entry = result["player_list"][0];
			entry["index"] = 0;
			entry["name"] = std::format("{}_player01", player->get_account_id());
			entry["espionage_lose"] = stats->get_fob_sneak_win();
			entry["espionage_win"] = stats->get_fob_sneak_lose();
			entry["fob_grade"] = stats->get_fob_grade();
			entry["fob_point"] = stats->get_fob_point();
			entry["fob_rank"] = stats->get_fob_rank();
			entry["is_insurance"] = std::uint8_t(stats->has_insurance());
			entry["league_grade"] = stats->get_league_grade();
			entry["league_rank"] = stats->get_league_rank();
			entry["playtime"] = stats->get_playtime();
			entry["point"] = stats->get_point();
			result["player_num"] = 1;
		}
		else
		{
			result["player_list"] = nlohmann::json::array();
			result["player_num"] = 0;
		}

		return result;
	}

	bool cmd_get_playerlist::needs_player()
	{
		return true;
	}
}
