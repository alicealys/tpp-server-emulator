#include <std_include.hpp>

#include "mgo_stat.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::mgo_stat
{
	std::vector<std::uint32_t> get_id_list()
	{
		// todo: reverse the value structure
		static const std::vector<std::uint32_t> list =
		{
			914658497,
			956134950,
			1170814218,
			1274279971,
			1360877150,
			1810991501,
			1939875265,
			2228647481,
			2279509158,
			2973130442,
			3001637186,
			3220552451,
			3391131435,
			3685558784,
			3743820509
		};

		return list;
	}

	std::unordered_set<std::uint32_t> get_id_set()
	{
		// todo: reverse the value structure
		static const std::unordered_set<std::uint32_t> list =
		{
			914658497,
			956134950,
			1170814218,
			1274279971,
			1360877150,
			1810991501,
			1939875265,
			2228647481,
			2279509158,
			2973130442,
			3001637186,
			3220552451,
			3391131435,
			3685558784,
			3743820509
		};

		return list;
	}

	GET_FIELD_C(mgo_stat, std::uint64_t, id);
	GET_FIELD_C(mgo_stat, std::uint64_t, player_id);
	GET_FIELD_C(mgo_stat, std::uint32_t, stat_id);
	GET_FIELD_C(mgo_stat, std::uint32_t, stat_value);

	namespace impl
	{
		template <database_type_t Type>
		std::uint32_t get_stat(const std::uint64_t player_id, const std::uint32_t stat_id)
		{
			return database::access<std::uint32_t>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_stat::table.stat_value)
							.from(mgo_stat::table)
								.where(mgo_stat::table.player_id == player_id && mgo_stat::table.stat_id == stat_id));

				if (results.empty())
				{
					return 0u;
				}

				return static_cast<std::uint32_t>(results.front().stat_value.value());
			});
		}

		template <database_type_t Type>
		bool set_stat(const std::uint64_t player_id, const std::uint32_t stat_id, const std::uint32_t stat_value)
		{
			return database::access<bool>([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(mgo_stat::table.stat_value)
							.from(mgo_stat::table)
								.where(mgo_stat::table.player_id == player_id && mgo_stat::table.stat_id == stat_id));
				
				if (results.empty())
				{
					auto result = db.get_database<Type>()->operator()(
						sqlpp::insert_into(mgo_stat::table)
							.set(mgo_stat::table.player_id = player_id, mgo_stat::table.stat_id = stat_id, mgo_stat::table.stat_value = stat_value));
					return result != 0ull;
				}
				else
				{
					auto result = db.get_database<Type>()->operator()(
						sqlpp::update(mgo_stat::table)
							.set(mgo_stat::table.stat_value = stat_value)
								.where(mgo_stat::table.player_id == player_id && mgo_stat::table.stat_id == stat_id));
					return result != 0ull;
				}
			});
		}

		template <database_type_t Type>
		std::vector<mgo_stat> get_stats(const std::uint64_t player_id)
		{
			return database::access<std::vector<mgo_stat>>([&](database_t& db)
				-> std::vector<mgo_stat>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_stat::table))
							.from(mgo_stat::table)
								.where(mgo_stat::table.player_id == player_id));

				std::vector<mgo_stat> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}
	}

	std::uint32_t get_stat(const std::uint64_t player_id, const std::uint32_t stat_id)
	{
		RUN_IMPL(impl::get_stat, player_id, stat_id);
	}

	std::vector<mgo_stat> get_stats(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_stats, player_id);
	}

	bool set_stat(const std::uint64_t player_id, const std::uint32_t stat_id, const std::uint32_t stat_value)
	{
		RUN_IMPL(impl::set_stat, player_id, stat_id, stat_value);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_stat.create");
		}
	};
}

REGISTER_TABLE(database::mgo_stat::table, -1)
