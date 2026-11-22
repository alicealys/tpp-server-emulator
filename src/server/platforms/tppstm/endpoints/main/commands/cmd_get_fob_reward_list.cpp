#include <std_include.hpp>

#include "cmd_get_fob_reward_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"
#include "database/models/fob_rewards.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_reward_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["reward_info"] = nlohmann::json::array();

		const auto rewards = database::fob_rewards::get_rewards(player->get_id());
		database::fob_rewards::clear_rewards(player->get_id());

		for (auto i = 0ull; i < rewards.size(); i++)
		{
			auto& entry_j = result["reward_info"][i];

			auto& player_info_j = entry_j["player_info"];
			auto& reward_info_j = entry_j["reward_info"];
			auto& npid_j = player_info_j["npid"];

			npid_j["handler"]["data"] = "";
			npid_j["handler"]["dummy"] = {0, 0, 0};
			npid_j["handler"]["term"] = 0;
			npid_j["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
			npid_j["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};

			player_info_j["player_id"] = rewards[i].attacker_id;
			player_info_j["player_name"] = std::format("{}_player01", rewards[i].attacker_xuid);
			player_info_j["ugc"] = 0;
			player_info_j["xuid"] = rewards[i].attacker_xuid;

			for (auto o = 0ull; o < rewards[i].rewards.size(); o++)
			{
				rewards[i].rewards[o].to_json(reward_info_j[o]);
			}
		}

		result["reward_count"] = rewards.size();
		result["version"] = 0;

		return result;
	}

	bool cmd_get_fob_reward_list::needs_player()
	{
		return true;
	}
}
