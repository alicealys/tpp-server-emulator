#include <std_include.hpp>

#include "mgo_color_purchase.hpp"

#include <utils/string.hpp>

namespace database::mgo_color_purchase
{
	GET_FIELD_C(mgo_color_purchase, std::uint64_t, id);
	GET_FIELD_C(mgo_color_purchase, std::uint64_t, player_id);
	GET_FIELD_C(mgo_color_purchase, std::uint32_t, gear_id);
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
								.where(mgo_color_purchase::table.player_id == player_id && mgo_color_purchase::table.gear_id == gear_id));

				return !results.empty();
			});
		}

		template <database_type_t Type>
		bool buy_color(const std::uint64_t player_id, const std::uint32_t gear_id, const std::uint32_t color_id)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(mgo_color_purchase::table)
						.set(mgo_color_purchase::table.player_id = player_id, mgo_color_purchase::table.gear_id = gear_id,
							 mgo_color_purchase::table.color_id = color_id));
				return result != 0ull;
			});
		}

		template <database_type_t Type>
		std::unordered_set<std::uint32_t> get_purchased_colors(const std::uint64_t player_id, const std::uint32_t gear_id)
		{
			return database::access<std::unordered_set<uint32_t>>([&](database_t& db)
				-> std::unordered_set<uint32_t>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_color_purchase::table))
							.from(mgo_color_purchase::table)
								.where(mgo_color_purchase::table.player_id == player_id && mgo_color_purchase::table.gear_id == gear_id));

				std::unordered_set<uint32_t> list;

				for (const auto& row : results)
				{
					list.insert(static_cast<std::uint32_t>(row.color_id.value()));
				}

				return list;
			});
		}
	}

	bool has_gear(const std::uint64_t player_id, const std::uint32_t gear_id)
	{
		RUN_IMPL(impl::has_gear, player_id, gear_id);
	}

	bool buy_color(const std::uint64_t player_id, const std::uint32_t gear_id, const std::uint32_t color_id)
	{
		RUN_IMPL(impl::buy_color, player_id, gear_id, color_id);
	}

	std::unordered_set<std::uint32_t> get_purchased_colors(const std::uint64_t player_id, const std::uint32_t gear_id)
	{
		RUN_IMPL(impl::get_purchased_colors, player_id, gear_id);
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
