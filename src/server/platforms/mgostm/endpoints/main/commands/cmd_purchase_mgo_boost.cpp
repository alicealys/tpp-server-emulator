#include <std_include.hpp>

#include "cmd_purchase_mgo_boost.hpp"

#include "database/models/mgo_item_purchases.hpp"
#include "database/models/shop_purchases.hpp"
#include "database/models/player_data.hpp"
#include "database/models/mgo_data.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_purchase_mgo_boost::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& purchase_id_j = data["purchase_id"];
		if (!purchase_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto purchase_id = purchase_id_j.get<std::uint32_t>();
		const auto boost = database::mgo_item_purchases::get_purchaseable_boost(purchase_id);
		const auto mgo_data = database::mgo_data::find(player->get_id());

		if (!mgo_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		if (!boost.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		if (!database::player_data::spend_mb_coins(player->get_id(), boost->price))
		{
			return error(ERR_MBCOIN_SHORTAGE);
		}
		else
		{
			database::shop_purchases::add_spent_single(player->get_id(), database::shop_purchases::gp_boost, boost->price, boost->purchase_id);
			const auto expire = mgo_data->get_gp_boost_expire();
			const auto begin = expire > 0s
				? std::chrono::system_clock::time_point(expire)
				: std::chrono::system_clock::now();
			database::mgo_data::set_gp_boost(player->get_id(), boost->boost_mag, begin + boost->effect_seconds * 1s);
		}

		return result;
	}

	bool cmd_purchase_mgo_boost::needs_player()
	{
		return true;
	}
}
