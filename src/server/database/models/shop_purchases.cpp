#include <std_include.hpp>

#include "shop_purchases.hpp"
#include "player_data.hpp"

#include <utils/string.hpp>

namespace database::shop_purchases
{
	std::vector<shop_item_entry_t> load_shop_item_list()
	{
		std::vector<shop_item_entry_t> entries;

		const auto list = utils::resources::load_json(RESOURCE_SHOP_ITEM_LIST);

		for (auto i = 0ull; i < list.size(); i++)
		{
			shop_item_entry_t entry{};
			entry.item_type = list[i]["id"].get<std::uint32_t>();
			entry.name = list[i]["name"].get<std::string>();
			entry.price = list[i]["price"].get<std::uint32_t>();
			entries.emplace_back(entry);
		}

		return entries;
	}

	std::vector<shop_item_entry_t>& get_shop_item_list()
	{
		static auto entries = load_shop_item_list();
		return entries;
	}

	std::uint32_t get_waters_item_type(const std::uint32_t area_id)
	{
		const auto area_code = area_id - (area_id % 10);
		switch (area_code)
		{
		case game::ocean_area_0:
			return 0u;
		case game::ocean_area_10:
			return waters_mid_atlantic_ridge;
		case game::ocean_area_20:
			return waters_east_of_the_hawaii_islands;
		case game::ocean_area_30:
			return waters_south_atlantic_ocean;
		case game::ocean_area_40:
			return waters_indian_ocean;
		case game::ocean_area_50:
			return waters_north_pacific_ocean;
		case game::ocean_area_60:
			return waters_south_pacific_ocean;
		case game::ocean_area_70:
			return waters_north_atlantic_ocean;
		}

		return 0u;
	}

	GET_FIELD_C(shop_purchase, std::uint64_t, id);
	GET_FIELD_C(shop_purchase, std::uint64_t, player_id);
	GET_FIELD_C(shop_purchase, std::chrono::seconds, date);
	GET_FIELD_C(shop_purchase, std::chrono::seconds, expire_date);
	GET_FIELD_C(shop_purchase, std::uint32_t, event_type);
	GET_FIELD_C(shop_purchase, std::uint32_t, item_quantity);
	GET_FIELD_C(shop_purchase, std::uint32_t, item_type);
	GET_FIELD_C(shop_purchase, std::uint32_t, param1);
	GET_FIELD_C(shop_purchase, std::uint32_t, remaining_coin);
	GET_FIELD_C(shop_purchase, std::uint32_t, coin_quantity);

	namespace impl
	{
		template <database_type_t Type>
		bool add_entry(const std::uint64_t player_id, const entry_params_t& params)
		{
			const auto date = std::chrono::system_clock::now();
			const auto expire_date = date + 24h * 180;
			const auto remaining_coin = database::player_data::get_mb_coins(player_id);

			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(shop_purchase::table)
						.set(shop_purchase::table.player_id = player_id, shop_purchase::table.coin_quantity = params.coin_quantity,
							 shop_purchase::table.date = date, shop_purchase::table.expire_date = expire_date, shop_purchase::table.event_type = params.event_type,
							 shop_purchase::table.item_quantity = params.item_quantity, shop_purchase::table.item_type = params.item_type,
							 shop_purchase::table.param1 = params.param1, shop_purchase::table.remaining_coin = remaining_coin));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::vector<shop_purchase> get_history(const std::uint64_t player_id, const std::uint32_t limit)
		{
			return database::access<std::vector<shop_purchase>>([&](database_t& db)
				-> std::vector<shop_purchase>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(shop_purchase::table))
							.from(shop_purchase::table)
								.where(shop_purchase::table.player_id == player_id)
									.order_by(shop_purchase::table.date.desc()).limit(limit));

				std::vector<shop_purchase> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::size_t get_history_size(const std::uint64_t player_id)
		{
			return database::access<std::size_t>([&](database_t& db)
				-> std::size_t
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::count(1))
							.from(shop_purchase::table)
								.where(shop_purchase::table.player_id == player_id));
				
				if (results.empty())
				{
					return 0u;
				}

				return static_cast<std::size_t>(results.front().count);
			});
		}
	}

	bool add_entry(const std::uint64_t player_id, const entry_params_t& params)
	{
		RUN_IMPL(impl::add_entry, player_id, params);
	}

	bool add_spent_single(const std::uint64_t player_id, const std::uint32_t item_type, const std::uint32_t price, 
		const std::uint32_t param1)
	{
		entry_params_t params{};
		params.item_quantity = 1u;
		params.item_type = item_type;
		params.coin_quantity = price;
		params.event_type = 1u;
		params.param1 = param1;

		return add_entry(player_id, params);
	}

	std::size_t get_history_size(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_history_size, player_id);
	}

	std::vector<shop_purchase> get_history(const std::uint64_t player_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_history, player_id, limit);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.shop_purchases.create");
		}
	};
}

REGISTER_TABLE(database::shop_purchases::table, -1)
