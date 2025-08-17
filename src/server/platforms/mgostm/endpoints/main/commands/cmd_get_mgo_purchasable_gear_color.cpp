#include <std_include.hpp>

#include "database/models/mgo_color_purchase.hpp"

#include "cmd_get_mgo_purchasable_gear_color.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_purchasable_gear_color::cmd_get_mgo_purchasable_gear_color()
	{
		this->list_ = utils::resources::load_json(RESOURCE_MGO_GEAR_COLORS);
	}

	nlohmann::json cmd_get_mgo_purchasable_gear_color::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& gear_id_j = data["gear_id"];
		if (!gear_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto gear_id = gear_id_j.get<std::uint32_t>();

		result["purchasable_gear_color"]["already_released"] = 1;
		result["purchasable_gear_color"]["gear_id"] = gear_id;
		result["purchasable_gear_color"]["purchasable_color_list"] = this->list_;

		const auto purchased_colors = database::mgo_color_purchase::get_purchased_colors(player->get_id(), gear_id);

		auto& list = result["purchasable_gear_color"]["purchasable_color_list"];
		for (auto i = 0ull; i < this->list_.size(); i++)
		{
			const auto color_id = list[i]["color"].get<std::uint32_t>();
			list[i]["already_purchased"] = purchased_colors.contains(color_id);
		}

		return result;
	}

	bool cmd_get_mgo_purchasable_gear_color::needs_player()
	{
		return true;
	}
}
