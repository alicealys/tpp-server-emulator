#include <std_include.hpp>

#include "table_loader.hpp"

namespace database
{
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
}
