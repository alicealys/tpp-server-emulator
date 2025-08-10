#include <std_include.hpp>

#include "table_loader.hpp"

namespace database
{
	namespace
	{
		using queries_t = std::unordered_map<std::string, std::string>;

		queries_t database_queries[database::database_count];

		void parse_sql_file(const database::database_type_t type, const std::string& data)
		{
			auto& map = database_queries[type];

			std::string current_query_name;
			std::string current_query;
			auto is_in_query = false;

			char query_name_prefix[] = "-- query:";

			const auto lines = utils::string::split(data, '\n');
			for (const auto& line : lines)
			{
				if (line.starts_with(query_name_prefix))
				{
					if (is_in_query)
					{
						map.insert(std::make_pair(current_query_name, current_query));
						current_query.clear();
						is_in_query = false;
					}

					is_in_query = true;
					current_query_name = utils::string::trim(line.substr(sizeof(query_name_prefix) - 1));
				}
				else
				{
					current_query.append(line);
					current_query.append("\n");
				}
			}

			if (is_in_query)
			{
				map.insert(std::make_pair(current_query_name, current_query));
				current_query.clear();
			}
		}

		void load_sql_file()
		{
			static auto done = false;
			if (done)
			{
				return;
			}

			done = true;

			parse_sql_file(database::database_mysql, utils::resources::load(RESOURCE_SQL_MYSQL));
			parse_sql_file(database::database_sqlite3, utils::resources::load(RESOURCE_SQL_SQLITE3));
		}
	}

	tables& get_tables()
	{
		static tables tables = {};
		return tables;
	}

	void register_table(std::unique_ptr<table_interface>&& table, int priority)
	{
		table_def def{};
		def.priority = priority;
		def.inst = std::move(table);

		auto& tables = get_tables();
		tables.push_back(std::move(def));

		std::sort(tables.begin(), tables.end(), [](const auto& a, const auto& b)
		{
			return a.priority > b.priority;
		});
	}

	std::string get_sql_query(const database::database_type_t type, const std::string& name)
	{
		load_sql_file();

		auto& map = database_queries[type];
		const auto& iter = map.find(name);
		if (iter == map.end())
		{
			throw std::runtime_error(std::format("query not found {}", name));
		}

		return iter->second;
	}
}
