#include <std_include.hpp>

#include "mgo_color_purchase.hpp"

#include <utils/string.hpp>

namespace database::mgo_color_purchase
{
	std::unordered_map<std::uint32_t, item_info_t> load_gear_info_map()
	{
		std::unordered_map<std::uint32_t, item_info_t> map;

		const auto list = utils::resources::load_json(RESOURCE_MGO_GEAR_LIST);
		for (auto i = 0ull; i < list.size(); i++)
		{
			item_info_t info{};
			info.category = gear;
			info.id = list[i]["gear_id"].get<std::uint32_t>();
			info.default_color = list[i]["default_color"].get<std::uint32_t>();
			info.point = list[i]["point"].get<std::uint32_t>();
			info.purchase_type = list[i]["purchase_type"].get<std::uint32_t>();
			info.prestige = list[i]["prestige"].get<std::uint32_t>();
			map.insert(std::make_pair(info.id, info));
		}

		return map;
	}

	std::vector<color_t> load_gear_color_list()
	{
		std::vector<color_t> list;

		const auto colors = utils::resources::load_json(RESOURCE_MGO_GEAR_COLORS);
		for (auto i = 0ull; i < colors.size(); i++)
		{
			color_t color{};
			color.color = colors[i]["color"].get<std::uint32_t>();
			color.point = colors[i]["point"].get<std::uint32_t>();
			color.purchase_type = colors[i]["purchase_type"].get<std::uint32_t>();
			color.level = colors[i]["level"].get<std::uint32_t>();
			color.prestige = colors[i]["prestige"].get<std::uint32_t>();
			list.emplace_back(color);
		}

		return list;
	}

	std::vector<color_t> load_weapon_color_list()
	{
		std::vector<color_t> list;

		const auto colors = utils::resources::load_json(RESOURCE_MGO_WEAPON_COLORS);
		for (auto i = 0ull; i < colors.size(); i++)
		{
			color_t color{};
			color.color = colors[i]["color"].get<std::uint32_t>();
			color.point = colors[i]["point"].get<std::uint32_t>();
			color.purchase_type = colors[i]["purchase_type"].get<std::uint32_t>();
			color.level = colors[i]["level"].get<std::uint32_t>();
			color.prestige = colors[i]["prestige"].get<std::uint32_t>();
			list.emplace_back(color);
		}

		return list;
	}

	std::unordered_map<std::uint32_t, item_info_t>& get_gear_info_map()
	{
		static auto map = load_gear_info_map();
		return map;
	}

	std::vector<color_t>& get_gear_colors()
	{
		static auto list = load_gear_color_list();
		return list;
	}

	std::vector<color_t>& get_weapon_colors()
	{
		static auto list = load_weapon_color_list();
		return list;
	}

	item_info_t get_gear_info(const std::uint32_t gear_id)
	{
		const auto& map = get_gear_info_map();
		const auto iter = map.find(gear_id);

		if (iter != map.end())
		{
			return iter->second;
		}

		item_info_t info{};
		info.category = gear;
		info.id = gear_id;
		info.default_color = 0;
		info.point = 10;
		info.default_color = 288703677;
		info.purchase_type = 2;
		info.is_default = true;

		return info;
	}

	std::optional<color_t> get_gear_color(const std::uint32_t color_id)
	{
		const auto& list = get_gear_colors();
		const auto iter = std::ranges::find_if(list.begin(), list.end(), [&](const color_t& color)
		{
			return color.color == color_id;
		});

		if (iter != list.end())
		{
			return {*iter};
		}

		return {};
	}

	std::optional<color_t> get_weapon_color(const std::uint32_t color_id)
	{
		const auto& list = get_weapon_colors();
		const auto iter = std::ranges::find_if(list.begin(), list.end(), [&](const color_t& color)
		{
			return color.color == color_id;
		});

		if (iter != list.end())
		{
			return {*iter};
		}

		return {};
	}

	GET_FIELD_C(mgo_color_purchase, std::uint64_t, id);
	GET_FIELD_C(mgo_color_purchase, std::uint64_t, player_id);
	GET_FIELD_C(mgo_color_purchase, std::uint32_t, category);
	GET_FIELD_C(mgo_color_purchase, std::uint32_t, item_id);
	GET_FIELD_C(mgo_color_purchase, std::uint32_t, color_id);

	namespace impl
	{
		template <database_type_t Type>
		bool has_gear(const std::uint64_t player_id, const std::uint32_t gear_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_color_purchase::table.color_id)
							.from(mgo_color_purchase::table)
								.where(mgo_color_purchase::table.player_id == player_id && mgo_color_purchase::table.category == static_cast<std::uint32_t>(gear) && 
									   mgo_color_purchase::table.item_id == gear_id));

				return !results.empty();
			});
		}

		template <database_type_t Type>
		bool buy_color(const std::uint64_t player_id, const item_category category, const std::uint32_t item_id, const std::uint32_t color_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(mgo_color_purchase::table)
						.set(mgo_color_purchase::table.player_id = player_id, mgo_color_purchase::table.category = static_cast<std::uint32_t>(category),
							 mgo_color_purchase::table.item_id = item_id, mgo_color_purchase::table.color_id = color_id));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::unordered_set<std::uint32_t> get_purchased_colors(const std::uint64_t player_id, const item_category category, const std::uint32_t item_id)
		{
			return database::access<std::unordered_set<uint32_t>>([&](database_t& db)
				-> std::unordered_set<uint32_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_color_purchase::table))
							.from(mgo_color_purchase::table)
								.where(mgo_color_purchase::table.player_id == player_id && 
									   mgo_color_purchase::table.category == static_cast<std::uint32_t>(category) && 
									   mgo_color_purchase::table.item_id == item_id));

				std::unordered_set<uint32_t> list;

				for (const auto& row : results)
				{
					list.insert(static_cast<std::uint32_t>(row.color_id.value()));
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::vector<mgo_color_purchase> get_all_purchased_colors(const std::uint64_t player_id, const item_category category)
		{
			return database::access<std::vector<mgo_color_purchase>>([&](database_t& db)
				-> std::vector<mgo_color_purchase>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_color_purchase::table))
							.from(mgo_color_purchase::table)
								.where(mgo_color_purchase::table.player_id == player_id &&
									   mgo_color_purchase::table.category == static_cast<std::uint32_t>(category)));

				std::vector<mgo_color_purchase> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}
	}

	bool has_gear(const std::uint64_t player_id, const std::uint32_t gear_id)
	{
		RUN_IMPL(impl::has_gear, player_id, gear_id);
	}

	bool buy_color(const std::uint64_t player_id, const item_category category, const std::uint32_t gear_id, const std::uint32_t color_id)
	{
		RUN_IMPL(impl::buy_color, player_id, category, gear_id, color_id);
	}

	std::unordered_set<std::uint32_t> get_purchased_colors(const std::uint64_t player_id, const item_category category, const std::uint32_t item_id)
	{
		RUN_IMPL(impl::get_purchased_colors, player_id, category, item_id);
	}

	std::vector<mgo_color_purchase> get_all_purchased_colors(const std::uint64_t player_id, const item_category category)
	{
		RUN_IMPL(impl::get_all_purchased_colors, player_id, category);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_color_purchase.create");
		}
	};
}

REGISTER_TABLE(database::mgo_color_purchase::table, -1)
