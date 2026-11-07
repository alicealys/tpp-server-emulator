#include <std_include.hpp>

#include "cmd_get_server_item.hpp"

#include "database/models/items.hpp"
#include "database/models/players.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_server_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& item_id_j = data["item_id"];
		if (!item_id_j.is_number_integer())
		{
			return error(ERR_INVALIDARG);
		}

		const auto item_type = item_id_j.get<std::uint32_t>();
		const auto item_status = database::items::get_item(player->get_id(), item_type);
		item_status.to_json(result["item"]);

		return result;
	}

	bool cmd_get_server_item::needs_player()
	{
		return true;
	}
}
