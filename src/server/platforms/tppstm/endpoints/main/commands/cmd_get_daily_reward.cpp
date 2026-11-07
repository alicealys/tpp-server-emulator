#include <std_include.hpp>

#include "cmd_get_daily_reward.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	std::vector<game::reward_t> cmd_get_daily_reward::daily_rewards_;

	cmd_get_daily_reward::cmd_get_daily_reward()
	{
		auto list = utils::resources::load_json(RESOURCE_DAILY_REWARDS);
		for (auto i = 0u; i < list.size(); i++)
		{
			auto& entry = list[i];
			if (!entry.is_object())
			{
				continue;
			}

			game::reward_t reward{};
			if (!reward.parse(entry))
			{
				throw std::runtime_error(std::format("[daily rewards] invalid reward \"{}\"", entry.dump()));
			}

			cmd_get_daily_reward::daily_rewards_.emplace_back(reward);
		}
	}

	game::reward_t& cmd_get_daily_reward::get_current_reward()
	{
		const auto now = std::chrono::system_clock::now();
		const auto day = std::chrono::duration_cast<std::chrono::days>(now.time_since_epoch()).count();
		const auto idx = day % cmd_get_daily_reward::daily_rewards_.size();
		return cmd_get_daily_reward::daily_rewards_[idx];
	}

	void cmd_get_daily_reward::give_daily_reward(const database::players::player& player)
	{
		const auto player_record = database::player_records::find(player.get_id());
		if (!player_record.has_value())
		{
			return;
		}

		const auto player_data = database::player_data::find(player.get_id());
		if (!player_data.has_value())
		{
			return;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - player_record->get_daily_last();

		if (diff > 24h)
		{
			database::player_records::set_daily_login(player.get_id());

			const auto& daily_reward = cmd_get_daily_reward::get_current_reward();
			player_data->give_reward(daily_reward);
		}
	}

	nlohmann::json cmd_get_daily_reward::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["count"] = 0;
		result["league_list"] = nlohmann::json::array();
		result["league_num"] = 0;
		result["personal_list"] = nlohmann::json::array();
		result["personal_num"] = 0;
		result["reward_type"] = 0;
		result["section"] = 0;

		const auto player_record = database::player_records::find(player->get_id());
		if (!player_record.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - player_record->get_daily_last_ack();

		cmd_get_daily_reward::give_daily_reward(player.value());

		if (diff > 24h)
		{
			database::player_records::set_daily_reward(player->get_id());

			const auto& daily_reward = cmd_get_daily_reward::get_current_reward();

			result["count"] = daily_reward.value;
			result["reward_type"] = daily_reward.type;
		}

		return result;
	}

	bool cmd_get_daily_reward::needs_player()
	{
		return true;
	}
}
