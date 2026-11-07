#include <std_include.hpp>

#include "cmd_send_online_challenge_task_status.hpp"
#include "cmd_get_daily_reward.hpp"

#include "database/models/player_records.hpp"
#include "database/models/player_data.hpp"
#include "database/models/challenge_tasks.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_send_online_challenge_task_status::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& version_j = data["version"];
		auto& status_list_j = data["status_list"];
		if (!version_j.is_number_unsigned() || !status_list_j.is_array())
		{
			return error(ERR_INVALIDARG);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto version = version_j.get<std::uint32_t>();
		const auto current_version = database::challenge_tasks::get_current_version();

		std::chrono::seconds start{};
		std::chrono::seconds end{};

		const auto current_tasks = database::challenge_tasks::get_current_challenge_tasks(start, end);
		const auto orders = database::challenge_tasks::get_order_list(player->get_id());

		auto& result_status_list_j = result["result_status_list"];
		result_status_list_j = nlohmann::json::array();

		const auto count = std::min(status_list_j.size(), current_tasks.size());
		for (auto i = 0u; i < count; i++)
		{
			auto status = 0u;

			if (version == current_version)
			{
				status = status_list_j[i].get<std::uint32_t>() & 0b11;

				const auto& task = current_tasks[i];
				const auto iter = std::ranges::find_if(orders.begin(), orders.end(),
					[&](const database::challenge_tasks::challenge_task_order& order)
					{
						return current_tasks[i].id == order.get_task_id();
					}
				);

				if (status == 2)
				{
					status = 3;
				}

				auto prev_status = 0u;
				if (iter == orders.end())
				{
					prev_status = 0u;
					database::challenge_tasks::create_order(player->get_id(), task.id, status, end);
				}
				else
				{
					prev_status = iter->get_status();
					database::challenge_tasks::set_order_status(iter->get_order_id(), status);
				}

				if (prev_status != 3 && status == 3)
				{
					player_data->give_reward(task.reward);
				}
			}

			result_status_list_j[i] = status;
		}

		return result;
	}

	bool cmd_send_online_challenge_task_status::needs_player()
	{
		return true;
	}
}
