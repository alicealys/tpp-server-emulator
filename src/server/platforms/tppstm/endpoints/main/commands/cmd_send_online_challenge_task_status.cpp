#include <std_include.hpp>

#include "cmd_send_online_challenge_task_status.hpp"

#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_send_online_challenge_task_status::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& status_list_j = data["status_list"];
		if (!status_list_j.is_array())
		{
			return error(ERR_INVALIDARG);
		}

		auto record = database::player_records::find(player->get_id());
		if (!record.has_value())
		{
			return error(ERR_DATABASE);
		}

		auto& challenge_tasks = record->get_challenge_tasks();

		for (auto i = 0; i < database::player_records::challenge_tasks_count; i++)
		{
			if (status_list_j[i].is_number_unsigned())
			{
				const auto flags = status_list_j[i].get<std::uint32_t>();
				if ((flags & 2) != 0)
				{
					challenge_tasks.status[i] |= 2;
					challenge_tasks.status[i] |= 1;
				}
			}
		}

		if (!database::player_records::set_challenge_tasks(player->get_id(), challenge_tasks))
		{
			return error(ERR_DATABASE);
		}

		result["result_status_list"] = nlohmann::json::array();
		for (auto i = 0; i < database::player_records::challenge_tasks_count; i++)
		{
			result["result_status_list"][i] = challenge_tasks.status[i];
		}

		return result;
	}

	bool cmd_send_online_challenge_task_status::needs_player()
	{
		return true;
	}
}
