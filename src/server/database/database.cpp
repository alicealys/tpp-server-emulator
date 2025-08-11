#include <std_include.hpp>

#include "database.hpp"
#include "models/players.hpp"

#include "utils/config.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace database
{
	namespace
	{
		database_def_t database_def{};
	}

	std::array<connection_t, max_connections> connection_pool;

	database_type_t get_database_type()
	{
		return database_def.type;
	}

	void set_database_type(const database_type_t type)
	{
		database_def.type = type;

		switch (type)
		{
		case database_mysql:
		{
			database_def.rand_func = "rand()";
			database_def.use_multi_connection = true;
			break;
		}
		case database_sqlite3:
		{
			database_def.rand_func = "random()";
			database_def.use_multi_connection = false;
			break;
		}
		}
	}

	database_def_t get_database_def()
	{
		return database_def;
	}

	database_type_t get_database_type(const std::string& type_name)
	{
		static std::unordered_map<std::string, database_type_t> map =
		{
			{"mysql", database_mysql},
			{"sqlite3", database_sqlite3},
		};

		const auto lower = utils::string::to_lower(type_name);
		const auto iter = map.find(lower);
		if (iter == map.end())
		{
			return database_invalid;
		}

		return iter->second;
	}

	database_config load_config()
	{
		const auto type_name = config::get<std::string>("database_type");
		const auto type = get_database_type(type_name);

		if (type == database_invalid)
		{
			throw std::runtime_error(std::format("invalid database type specified: {}", type_name));
		}

		set_database_type(type);

		database_config config;
		config.user = config::get<std::string>("database_user");
		config.password = config::get<std::string>("database_password");
		config.host = config::get<std::string>("database_host");
		config.port = config::get<std::uint16_t>("database_port");
		config.database_name = config::get<std::string>("database_name");
		return config;
	}

	database_config& get_config()
	{
		static auto config = load_config();
		return config;
	}

	sqlpp::mysql::connection* database_container::get_mysql() const
	{
		return this->dbs_.mysql_.get();
	}

	sqlpp::sqlite3::connection* database_container::get_sqlite3() const
	{
		return this->dbs_.sqlite3_.get();
	}

	void database_container::run_query(const std::string& name)
	{
		this->execute(get_sql_query(database::get_database_type(), name));
	}

	size_t database_container::execute(const std::string& query)
	{
		if (get_database_type() == database_mysql)
		{
			this->dbs_.mysql_->execute(query);
			return 0ull;
		}

		if (get_database_type() == database_sqlite3)
		{
			return this->dbs_.sqlite3_->execute(query);
		}

		throw std::runtime_error("[database_container::execute] invalid database provider");
	}

	void database_container::create_connection()
	{
		const auto& base_config = get_config();

		switch (get_database_type())
		{
		case database_mysql:
		{
			sqlpp::mysql::connection_config config;
			config.user = base_config.user;
			config.password = base_config.password;
			config.host = base_config.host;
			config.port = base_config.port;
			config.database = base_config.database_name;

			this->dbs_.mysql_ = std::make_unique<sqlpp::mysql::connection>(config);
			return;
		}
		case database_sqlite3:
		{
			sqlpp::sqlite3::connection_config config;
			config.password = base_config.password;
			config.path_to_database = base_config.database_name;
			config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;

			this->dbs_.sqlite3_ = std::make_unique<sqlpp::sqlite3::connection>(config);
			return;
		}
		}

		throw std::runtime_error("[database_container::create_connection] invalid database type");
	}

	bool database_container::is_valid() const
	{
		if (get_database_type() == database_mysql)
		{
			return this->dbs_.mysql_.get() && this->dbs_.mysql_->is_valid();
		}

		if (get_database_type() == database_sqlite3)
		{
			return this->dbs_.sqlite3_.get();
		}

		return false;
	}

	void database_container::reset()
	{
		this->dbs_.mysql_.reset();
		this->dbs_.sqlite3_.reset();
	}

	void check_connection(connection_t& connection)
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto diff = now - connection.start;

		if (!connection.db.is_valid() || diff >= 1h)
		{
			connection.db.create_connection();
			connection.start = now;
		}

		connection.last_access = now;
	}

	void cleanup_connection(connection_t& connection)
	{
		std::unique_lock<database_mutex_t> lock(connection.mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			return;
		}

		const auto now = std::chrono::high_resolution_clock::now();
		const auto diff = now - connection.last_access;
		if (diff >= database::vars.session_timeout)
		{
			connection.db.reset();
		}
	}

	void cleanup_connections()
	{
		if (get_database_def().use_multi_connection)
		{
			for (auto& connection : connection_pool)
			{
				cleanup_connection(connection);
			}
		}
		else
		{
			auto& connection = connection_pool[SINGLE_CONNECTION_INDEX];
			cleanup_connection(connection);
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
				try
				{
					table.inst->run_tasks(db);
				}
				catch (const std::exception& e)
				{
					console::error("database::run_tasks: %s\n", e.what());
				}
			}
		});
	}

	void initialize()
	{
		initialize_vars();
		create_tables();
	}
}
