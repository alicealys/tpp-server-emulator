#include <std_include.hpp>

#include "cmd_get_wormhole_list.hpp"

#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_wormhole_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& flag_j = data["flag"];
		auto flag = database::wormholes::wormhole_flag_friendly;
		if (flag_j.is_string())
		{
			flag = database::wormholes::get_flag_id(flag_j.get<std::string>());
		}

		if (flag == database::wormholes::wormhole_flag_invalid)
		{
			return error(ERR_OPTION_OUTOFRANGE);
		}

		auto& wormhole_list_j = result["wormhole_list"];
		wormhole_list_j = nlohmann::json::array();

		auto count = 0u;
		const auto list = database::wormholes::get_wormhole_list(player->get_id(), flag);

		for (auto i = 0u; i < list.size(); i++)
		{
			if (!list[i].is_open())
			{
				continue;
			}

			const auto to_player = database::players::find(list[i].opponent_id);
			if (!to_player.has_value())
			{
				continue;
			}

			auto& entry = wormhole_list_j[count++];
			entry["flag"] = flag == database::wormholes::wormhole_flag_friendly ? 0 : 1;
			entry["left_hour"] = list[i].get_left_hour();
			entry["player_id"] = list[i].opponent_id;
			entry["player_name"] = std::format("{}_player01", to_player->get_account_id());
		}

		result["wormhole_num"] = count;

		return result;
	}

	bool cmd_get_wormhole_list::needs_player()
	{
		return true;
	}
}
