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
			return error(ERR_INVALID_SESSION);
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
			if (iter != server_item_list.end() && iter->second.get_develop() != database::items::developed)
			{
				check_result = 1;
				break;
			}
		}
		
		result["check_result"] = check_result;

		return error(NOERR);
	}
}
