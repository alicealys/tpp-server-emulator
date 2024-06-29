#pragma once

#include <utils/concurrency.hpp>
#include "table_loader.hpp"

#include "utils/config.hpp"
#include "vars.hpp"

namespace sql = sqlpp::mysql;

namespace database
{
	constexpr auto max_connections = 100;

	using database_mutex_t = std::recursive_mutex;

	struct connection_t
	{
		database_t db;
		database_mutex_t mutex;
		std::chrono::high_resolution_clock::time_point start;
		std::chrono::high_resolution_clock::time_point last_access;
	};

	extern std::array<connection_t, max_connections> connection_pool;

	void initialize();

	sql::connection_config& get_config();

	template <typename T = void, typename F>
	T access(F&& accessor)
	{
		for (auto& connection : connection_pool)
		{
			std::unique_lock<database_mutex_t> lock(connection.mutex, std::try_to_lock);
			if (!lock.owns_lock())
			{
				continue;
			}

			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - connection.start;

			if (!connection.db.get() || !connection.db->is_valid() || diff >= 1h)
			{
				connection.db = std::make_unique<sql::connection>(get_config());
				connection.start = now;
			}

			connection.last_access = now;
			return accessor(connection.db);
		}

		throw std::runtime_error("out of connections");
	}

	void cleanup_connections();

	void run_tasks();
}
