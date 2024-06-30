#include <std_include.hpp>

#include "cmd_purchase_online_development_completion.hpp"

#include "database/models/player_data.hpp"
#include "database/models/items.hpp"

namespace tpp
{
	nlohmann::json cmd_purchase_online_development_completion::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& develop_id_j = data["develop_id"];

		if (!develop_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto item_id = develop_id_j.get<std::uint32_t>();

		nlohmann::json result;

		const auto item = database::items::get_item(player->get_id(), item_id);
		if (item.get_develop() != database::items::indev)
		{
			return error(ERR_INVALIDARG);
		}

		if (database::player_data::spend_coins(player->get_id(), item.get_mb_coin()))
		{
			if (!database::items::force_develop(player->get_id(), item.get_id()))
			{
				database::player_data::add_coins(player->get_id(), item.get_mb_coin());
				return error(ERR_DATABASE);
			}
		}
		else
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}

		return error(NOERR);
	}

	bool cmd_purchase_online_development_completion::needs_player()
	{
		return true;
	}
}
