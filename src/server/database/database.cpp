#include <std_include.hpp>

#include "database.hpp"
#include "models/players.hpp"

#include "utils/config.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace database
{
	std::array<connection_t, max_connections> connection_pool;

	sql::connection_config load_config()
	{
		sql::connection_config config;
		config.user = config::get<std::string>("database_user");
		config.password = config::get<std::string>("database_password");
		config.host = config::get<std::string>("database_host");
		config.port = config::get<std::uint16_t>("database_port");
		config.database = config::get<std::string>("database_name");
		return config;
	}

	sql::connection_config& get_config()
	{
		static auto config = load_config();
		return config;
	}

	void cleanup_connections()
	{
		for (auto& connection : connection_pool)
		{
			std::unique_lock<database_mutex_t> lock(connection.mutex, std::try_to_lock);
			if (!lock.owns_lock())
			{
				continue;
			}

			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - connection.last_access;
			if (diff >= database::players::session_timeout)
			{
				connection.db.reset();
			}
		}
	}

	void create_tables()
	{
		database::access([](database_t& db)
		{
			for (const auto& table : get_tables())
			{
				table.inst->create(db);
			}
		});
	}

	void run_tasks()
	{
		database::access([](database_t& db)
		{
			for (const auto& table : get_tables())
			{
				table.inst->run_tasks(db);
			}
		});
	}
}
