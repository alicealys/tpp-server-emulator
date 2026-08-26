#include <std_include.hpp>

#include "mgo_titles.hpp"

#include <utils/string.hpp>

namespace database::mgo_titles
{
	std::vector<mgo_title_def_t> load_titles()
	{
		std::vector<mgo_title_def_t> result;

		const auto list = utils::resources::load_json(RESOURCE_MGO_TITLE_LIST);
		if (!list.is_array())
		{
			return result;
		}

		for (auto i = 0u; i < list.size(); i++)
		{
			mgo_title_def_t def{};
			def.title_id = list[i]["id"].get<std::uint32_t>();
			def.gp = list[i]["gp"].get<std::uint32_t>();
			result.emplace_back(def);
		}

		return result;
	}

	const std::vector<mgo_title_def_t>& get_titles()
	{
		static const auto titles = load_titles();
		return titles;
	}

	std::optional<mgo_title_def_t> get_title(const std::uint32_t id)
	{
		const auto& list = get_titles();
		const auto iter = std::ranges::find_if(list.begin(), list.end(), [&](const mgo_title_def_t& def)
		{
			return def.title_id == id;
		});

		if (iter == list.end())
		{
			return {};
		}

		return {*iter};
	}

	GET_FIELD_C(mgo_title, std::uint64_t, id);
	GET_FIELD_C(mgo_title, std::uint64_t, player_id);
	GET_FIELD_C(mgo_title, std::uint32_t, title_id);
	GET_FIELD_C(mgo_title, std::uint32_t, flag);
	GET_FIELD_C(mgo_title, std::uint32_t, gp);
	GET_FIELD_C(mgo_title, std::chrono::seconds, date);

	namespace impl
	{
		template <database_type_t Type>
		void set_player_title(const std::uint64_t player_id, const std::uint32_t title_id, const std::uint32_t flag, const std::uint32_t gp)
		{
			database::access([&](database_t& db)
			{
				const auto has_title =  db.get_database<Type>()->operator()(
					sqlpp::select(mgo_title::table.title_id)
						.from(mgo_title::table)
							.where(mgo_title::table.player_id == player_id && mgo_title::table.title_id == title_id));

				if (has_title.empty())
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(mgo_title::table)
							.set(mgo_title::table.flag = flag, mgo_title::table.gp = gp, mgo_title::table.date = std::chrono::system_clock::now())
								.where(mgo_title::table.player_id == player_id && mgo_title::table.title_id == title_id));
				}
				else
				{
					db.get_database<Type>()->operator()(
						sqlpp::insert_into(mgo_title::table)
							.set(mgo_title::table.player_id = player_id, 
								 mgo_title::table.title_id = title_id,
								 mgo_title::table.flag = flag,
								 mgo_title::table.gp = gp,
								 mgo_title::table.date = std::chrono::system_clock::now()
							));
				}
			});
		}

		template <database_type_t Type>
		std::vector<mgo_title> get_player_title_list(const std::uint64_t player_id)
		{
			return database::access<std::vector<mgo_title>>([&](database_t& db)
				-> std::vector<mgo_title>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(sqlpp::all_of(mgo_title::table))
							.from(mgo_title::table)
								.where(mgo_title::table.player_id == player_id));

				std::vector<mgo_title> list;

				for (const auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			return database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(mgo_title::table)
						.where(mgo_title::table.player_id == player_id));
			});
		}
	}

	void set_player_title(const std::uint64_t player_id, const std::uint32_t title_id, const std::uint32_t flag, const std::uint32_t gp)
	{
		RUN_IMPL(impl::set_player_title, player_id, title_id, flag, gp);
	}

	std::vector<mgo_title> get_player_title_list(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_player_title_list, player_id);
	}

	std::unordered_map<std::uint32_t, mgo_title> get_player_title_map(const std::uint64_t player_id)
	{
		std::unordered_map<std::uint32_t, mgo_title> map;

		const auto list = get_player_title_list(player_id);

		for (const auto& title : list)
		{
			map.insert(std::make_pair(title.get_title_id(), title));
		}

		return map;
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.mgo_titles.create");
		}
	};
}

REGISTER_TABLE(database::mgo_titles::table, -1)
