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

		const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch());

		const auto list = database::wormholes::get_wormholes_status(player->get_id(), flag);
		auto idx = 0;

		result["wormhole_list"] = nlohmann::json::array();

		for (auto i = 0u; i < list.size(); i++)
		{
			const auto to_player = database::players::find(list[i].to_player_id);
			if (!to_player.has_value())
			{
				continue;
			}

			auto& entry = result["wormhole_list"][idx++];
			entry["flag"] = flag == database::wormholes::wormhole_flag_friendly ? 0 : 1;
			entry["left_hour"] = std::chrono::duration_cast<std::chrono::seconds>(now - list[i].expire).count();
			entry["player_id"] = list[i].to_player_id;
			entry["player_name"] = std::format("{}_player01", to_player->get_account_id());
		}

		result["wormhole_num"] = idx;

		return result;
	}

	bool cmd_get_wormhole_list::needs_player()
	{
		return true;
	}
}
