#include <std_include.hpp>

#include "database.hpp"
#include "models/players.hpp"

#include "utils/config.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace database
{
	std::array<connection_t, max_connections> connection_pool;

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

	sql::connection_config load_config()
	{
		sql::connection_config config;
		config.user = config::get<std::string>("database_user").value();
		config.password = config::get<std::string>("database_password").value();
		config.host = config::get<std::string>("database_host").value();
		config.port = config::get<std::uint16_t>("database_port").value();
		config.database = config::get<std::string>("database_name").value();
		return config;
	}

	sql::connection_config& get_config()
	{
		static auto config = load_config();
		return config;
	}

	void connect(database_t& database)
	{
		try
		{
			database = std::make_unique<sql::connection>(get_config());
		}
		catch (const std::exception& e)
		{
			printf("[Database] Error connecting to database %s\n", e.what());
			throw std::runtime_error(e.what());
		}
	}

	connection_t& get_connection()
	{
		for (auto& connection : connection_pool)
		{
			if (connection.in_use || !connection.mutex.try_lock())
			{
				continue;
			}

			const auto _0 = gsl::finally([&]
			{
				connection.mutex.unlock();
			});

			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - connection.start;

			connection.in_use = true;
			if (!connection.db.get() || !connection.db->is_valid() || diff >= 1h)
			{
				connection.db = std::make_unique<sql::connection>(get_config());
				connection.start = now;
			}

			return connection;
		}

		throw std::runtime_error("out of connections");
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
		thread_local database_t database = {};

		if (database.get() == nullptr)
		{
			connect(database);
		}

		return database;
	}
}
