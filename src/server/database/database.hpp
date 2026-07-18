#pragma once

#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4267)
#pragma warning(disable: 4018)
#pragma warning(disable: 4996)
#include <sqlpp11/sqlpp11.h>
#ifdef MYSQL_SUPPORTED
#include <sqlpp11/mysql/mysql.h>
#endif
#include <sqlpp11/sqlite3/sqlite3.h>
#pragma warning(pop)

#include <utils/concurrency.hpp>

#include "utils/config.hpp"
#include "utils/resources.hpp"
#include "vars.hpp"

namespace database
{
	using database_mutex_t = std::recursive_mutex;

	enum database_type_t
	{
		database_invalid = 0,
		database_mysql = 1,
		database_sqlite3 = 2,
		database_count
	};

	struct database_def_t
	{
		database_type_t type;
		bool use_multi_connection;
		std::string rand_func;
	};

	struct database_config
	{
		std::string user;
		std::string password;
		std::string host;
		std::uint16_t port;
		std::string database_name;
	};

	database_type_t get_database_type();
	database_def_t get_database_def();
	std::string get_database_name();

	struct dummy_mysql_connection
	{
	};

#ifdef MYSQL_SUPPORTED
	using mysql_connection = sqlpp::mysql::connection;
#else
	using mysql_connection = dummy_mysql_connection;
#endif
	using sqlite3_connection = sqlpp::sqlite3::connection;

	class database_container
	{
	public:
		template <database_type_t Type, typename T = std::conditional<Type == database_mysql, mysql_connection*, sqlite3_connection*>::type>
		T get_database() const
		{
#ifdef MYSQL_SUPPORTED
			if constexpr (Type == database_mysql)
			{
				return this->dbs_.mysql_.get();
			}
#endif

			if constexpr (Type == database_sqlite3)
			{
				return this->dbs_.sqlite3_.get();
			}
		}

		mysql_connection* get_mysql() const;
		sqlite3_connection* get_sqlite3() const;

		bool is_valid() const;
		void create_connection();
		std::string get_sql_query(const std::string& name);

		template <typename... Args>
		void run_query(const std::string& name, Args&&... args)
		{
			const auto query = this->get_sql_query(name);
			const auto fmt = std::vformat(query, std::make_format_args(args...));
			this->execute(fmt);
		}

		size_t execute(const std::string& query);

		void reset();

	private:
		struct
		{
			std::unique_ptr<mysql_connection> mysql_{};
			std::unique_ptr<sqlite3_connection> sqlite3_{};
		} dbs_{};
	};

	using database_t = database_container;

	class connection
	{
	public:
		connection() = default;
		void check();
		void cleanup();

		database_t db;
		database_mutex_t mutex;

	private:
		std::chrono::high_resolution_clock::time_point start_;
		std::chrono::high_resolution_clock::time_point last_access_;

	};

	connection* get_connection(std::unique_lock<database_mutex_t>& lock);

	template <typename T = void, typename F>
	T access(F&& accessor)
	{
		std::unique_lock<database_mutex_t> lock;
		auto conn = get_connection(lock);
		if (conn == nullptr)
		{
			throw std::runtime_error("out of connections");
		}

		conn->check();
		return accessor(conn->db);
	}

	void initialize();
	void post_start();
	void run_tasks();
	void stop();

#ifdef MYSQL_SUPPORTED

#define SELECT_IMPL(__fn__) (get_database_type() == database_mysql ? __fn__<database_mysql> : __fn__<database_sqlite3>)

#define RUN_IMPL(__fn__, ...) \
	static auto fn = (get_database_type() == database_mysql ? __fn__<database_mysql> : __fn__<database_sqlite3>); \
	return fn(__VA_ARGS__); \

#define RUN_IMPL_OVERLOAD(__fn__, __def__, ...) \
	static auto fn = (get_database_type() == database_mysql ? static_cast<__def__>(__fn__<database_mysql>): static_cast<__def__>(__fn__<database_sqlite3>)); \
	return fn(__VA_ARGS__); \

#else

#define SELECT_IMPL(__fn__) (__fn__<database_sqlite3>)

#define RUN_IMPL(__fn__, ...) \
	static auto fn = __fn__<database_sqlite3>; \
	return fn(__VA_ARGS__); \

#endif
}

#include "table_loader.hpp"
