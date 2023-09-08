#include <std_include.hpp>

#include "database.hpp"
#include "models/players.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace database
{
	std::string get_smart_device_id(const std::string& account_id)
	{
		std::string data;
		data.resize(80);
		return utils::cryptography::base64::encode(data);
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

	void connect(database_t& database)
	{
		sql::connection_config config;
		config.user = "root";
		config.password = "root";
		config.host = "localhost";
		config.port = 3306;
		config.database = "mgstpp";
#ifdef DEBUG
		config.debug = true;
#endif

		try
		{
			database = std::make_unique<sql::connection>(config);
		}
		catch (const std::exception& e)
		{
			printf("[Database] Error connecting to database %s\n", e.what());
		}
	}

	bool create_tables()
	{
		auto& db = database::get();
		if (db.get() == nullptr)
		{
			return false;
		}

		for (const auto& table : get_tables())
		{
			try
			{
				table.inst->create(db);

			}
			catch (const std::exception& e)
			{
				printf("%s\n", e.what());
				return false;
			}
		}

		return true;
	}

	database_t& get()
	{
		static thread_local database_t database = {};
		if (database.get() == nullptr)
		{
			connect(database);
		}

		return database;
	}
}
