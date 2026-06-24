#include <std_include.hpp>

#include "mgo_item_purchases.hpp"

#include <utils/string.hpp>

namespace database::mgo_item_purchases
{
	std::unordered_map<std::uint32_t, purchase_info_t> load_purchasable_item_map()
	{
		std::unordered_map<std::uint32_t, purchase_info_t> map;

		const auto list = utils::resources::load_json(RESOURCE_MGO_PURCHASABLE_LIST);
		const auto& item_list = list["purchasable_item_list"];

		for (auto i = 0ull; i < item_list.size(); i++)
		{
			purchase_info_t info{};
			info.purchase_id = item_list[i]["purchase_id"].get<std::uint32_t>();
			info.purchase_type = item_list[i]["purchase_type"].get<std::uint32_t>();
			info.price = item_list[i]["price"].get<std::uint32_t>();
			info.category = item_list[i]["category"].get<std::uint32_t>();
			map.insert(std::make_pair(info.purchase_id, info));
		}

		return map;
	}

	std::unordered_map<std::uint32_t, purchase_info_t>& get_purchasable_item_map()
	{
		static auto map = load_purchasable_item_map();
		return map;
	}

	std::optional<purchase_info_t> get_purchase_info(const std::uint32_t purchase_id)
	{
		const auto& map = get_purchasable_item_map();
		const auto iter = map.find(purchase_id);
		if (iter == map.end())
		{
			return {};
		}

		return iter->second;
	}

	namespace impl
	{
		template <database_type_t Type>
		bool purchase_item(const std::uint64_t player_id, const std::uint32_t purchase_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(mgo_item_purchase::table)
						.set(mgo_item_purchase::table.player_id = player_id, mgo_item_purchase::table.purchase_id = purchase_id));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::vector<std::uint32_t> get_purchase_list(const std::uint64_t player_id)
		{
			return database::access<std::vector<std::uint32_t>>([&](database_t& db)
				-> std::vector<std::uint32_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_item_purchase::table))
							.from(mgo_item_purchase::table)
								.where(mgo_item_purchase::table.player_id == player_id));

				std::vector<std::uint32_t> list;

				for (const auto& row : results)
				{
					list.emplace_back(static_cast<std::uint32_t>(row.purchase_id.value()));
				}

				return list;
			});
		}
	}

	bool purchase_item(const std::uint64_t player_id, const std::uint32_t purchase_id)
	{
		RUN_IMPL(impl::purchase_item, player_id, purchase_id);
	}

	std::vector<std::uint32_t> get_purchase_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_purchase_list, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_item_purchases.create");
		}
	};
}

REGISTER_TABLE(database::mgo_item_purchases::table, -1)
