#include <std_include.hpp>

#include "cmd_get_security_product_list.hpp"

#include "database/models/player_records.hpp"
#include "database/models/shop_purchases.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	namespace
	{
		void give_insurance(const std::uint64_t player_id, const std::uint32_t duration)
		{
			database::player_records::set_insurance(player_id, duration * 1h);
		}
	}

	std::vector<security_product_t> cmd_get_security_product_list::security_products_;

	cmd_get_security_product_list::cmd_get_security_product_list()
	{
		this->register_security_product_handler(database::shop_purchases::fob_defense_insurance_3_day_free, give_insurance);
		this->register_security_product_handler(database::shop_purchases::fob_defense_insurance_1_day, give_insurance);
		this->register_security_product_handler(database::shop_purchases::fob_defense_insurance_3_day, give_insurance);
		this->register_security_product_handler(database::shop_purchases::fob_defense_insurance_7_day, give_insurance);
		this->register_security_product_handler(database::shop_purchases::fob_defense_insurance_14_day, give_insurance);

		auto list = utils::resources::load_json(RESOURCE_SECURITY_PRODUCT_LIST);
		for (auto i = 0u; i < list.size(); i++)
		{
			security_product_t security_product{};
			security_product.interval_hour = list[i]["interval_hour"].get<std::uint32_t>();
			security_product.period_hour = list[i]["period_hour"].get<std::uint32_t>();
			security_product.price = list[i]["price"].get<std::uint32_t>();
			security_product.is_free = list[i]["is_free"].get<std::uint32_t>() == 1;
			security_product.product_id = list[i]["product_id"].get<std::uint32_t>();

			if (const auto iter = this->security_products_handlers_.find(security_product.product_id); 
				iter != this->security_products_handlers_.end())
			{
				security_product.handler.emplace(iter->second);
			}

			cmd_get_security_product_list::security_products_.emplace_back(security_product);
		}
	}

	void cmd_get_security_product_list::register_security_product_handler(const std::uint32_t product_id, const security_product_handler_t& handler)
	{
		this->security_products_handlers_.insert(std::make_pair(product_id, handler));
	}

	std::optional<security_product_t> cmd_get_security_product_list::get_security_product(const std::uint32_t product_id)
	{
		const auto iter = std::ranges::find_if(cmd_get_security_product_list::security_products_, [&](const security_product_t& product)
		{
			return product.product_id == product_id;
		});

		if (iter == cmd_get_security_product_list::security_products_.end())
		{
			return {};
		}

		return {*iter};
	}

	bool cmd_get_security_product_list::can_purchase_any_security_product(const std::uint64_t player_id)
	{
		const auto last_purchase = database::shop_purchases::get_last_item_purchase_range(player_id,
			database::shop_purchases::fob_defense_insurance_beg, database::shop_purchases::fob_defense_insurance_end);

		if (!last_purchase.has_value())
		{
			return true;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - last_purchase->get_date();
		const auto last_product_used = cmd_get_security_product_list::get_security_product(last_purchase->get_item_type());
		if (!last_product_used.has_value())
		{
			return true;
		}

		return diff > 1h * last_product_used->interval_hour;
	}

	bool cmd_get_security_product_list::can_purchase_security_product(const std::uint64_t player_id, const security_product_t& product)
	{
		const auto last_purchase = database::shop_purchases::get_last_item_purchase(player_id, product.product_id);

		if (!last_purchase.has_value())
		{
			return true;
		}

		if (product.is_free && last_purchase.has_value())
		{
			return false;
		}

		const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		const auto diff = now - last_purchase->get_date();

		return diff > 1h * product.interval_hour;
	}

	bool cmd_get_security_product_list::purchase_security_product(const std::uint64_t player_id, const security_product_t& product)
	{
		if (!database::player_data::spend_mb_coins(player_id, product.price))
		{
			return false;
		}

		database::shop_purchases::add_spent_single(player_id, product.product_id, product.price, product.period_hour);

		if (product.handler.has_value())
		{
			product.handler->operator()(player_id, product.period_hour);
		}

		return true;
	}

	nlohmann::json cmd_get_security_product_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["list"] = nlohmann::json::array();

		auto idx = 0;
		for (const auto& product : cmd_get_security_product_list::security_products_)
		{
			if (!cmd_get_security_product_list::can_purchase_security_product(player->get_id(), product))
			{
				continue;
			}

			auto& entry = result["list"][idx++];
			entry["interval_hour"] = product.interval_hour;
			entry["period_hour"] = product.period_hour;
			entry["is_free"] = product.is_free ? 1 : 0;
			entry["price"] = product.price;
			entry["product_id"] = product.product_id;
		}

		return result;
	}

	bool cmd_get_security_product_list::needs_player()
	{
		return true;
	}
}
