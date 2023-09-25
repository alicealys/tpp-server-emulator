#include <std_include.hpp>

#include "cmd_check_server_item_correct.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_check_server_item_correct::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& item_list = data["item_list"];
		if (!item_list.is_array())
		{
			return result;
		}

		auto check_result = 0;

		const auto server_item_list = database::items::get_item_list(player->get_id());
		for (auto i = 0; i < item_list.size(); i++)
		{
			if (!item_list[i].is_number_unsigned())
			{
				continue;
			}

			const auto item_id = item_list[i].get<std::uint32_t>();
			const auto& iter = server_item_list.find(item_id);
			if (iter != server_item_list.end() && iter->second.get_develop() != 2)
			{
				check_result = 1;
				break;
			}
		}

		if (check_result == 0)
		{
			const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());
			if (active_sneak.has_value())
			{
				database::players::set_active_sneak(player->get_id(), active_sneak->get_fob_id(), active_sneak->get_owner_id(), active_sneak->get_platform(),
					active_sneak->get_mode(), database::players::status_in_game, active_sneak->is_sneak());
			}
		}

		result["check_result"] = check_result;

		return result;
	}
}
