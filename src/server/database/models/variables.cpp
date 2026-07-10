#include <std_include.hpp>

#include "variables.hpp"

namespace database::variables
{
	namespace
	{
		std::unordered_set<std::string> lock_names;
	}

	namespace impl
	{
		template <database_type_t Type>
		void set(const std::string& name, const nlohmann::json& value)
		{
			return database::access([&](database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(variable::table.variable_name)
							.from(variable::table)
								.where(variable::table.variable_name == name));
				
				if (results.empty())
				{
					db.get_database<Type>()->operator()(
						sqlpp::insert_into(variable::table)
							.set(variable::table.variable_name = name, variable::table.variable_value = value.dump()));
				}
				else
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(variable::table)
							.set(variable::table.variable_value = value.dump())
								.where(variable::table.variable_name == name));
				}
			});
		}

		template <database_type_t Type>
		std::optional<nlohmann::json> get(const std::string& name)
		{
			return database::access<std::optional<nlohmann::json>>([&](database_t& db)
				-> std::optional<nlohmann::json>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(variable::table.variable_value)
							.from(variable::table)
								.where(variable::table.variable_name == name));

				if (results.empty())
				{
					return {};
				}

				return {nlohmann::json::parse(results.front().variable_value.value())};
			});
		}

		template <database_type_t Type>
		void access_with_lock(const std::string& name, const std::function<void()>& callback)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->start_transaction();

				try
				{
					auto results = db.get_database<Type>()->operator()(
						sqlpp::select(variable::table.variable_name)
							.from(variable::table).for_update()
								.where(variable::table.variable_name == name));

					if (!results.empty())
					{
						callback();
					}
				}
				catch (const std::exception& e)
				{
					console::error("failed to acquire lock: %s\n", e.what());
				}

				db.get_database<Type>()->commit_transaction();
			});
		}
	}

	void set(const std::string& name, const nlohmann::json& value)
	{
		RUN_IMPL(impl::set, name, value);
	}

	std::optional<nlohmann::json> get(const std::string& name)
	{
		RUN_IMPL(impl::get, name);
	}

	void access_with_lock(const std::string& name, const std::function<void()>& callback)
	{
		RUN_IMPL(impl::access_with_lock, name, callback);
	}

	void register_lock(const std::string& name)
	{
		lock_names.insert(name);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.variables.create");
		}

		void post_start(database_t& database) override
		{
			for (const auto& name : lock_names)
			{
				set(name, "null");
			}
		}
	};
}

REGISTER_TABLE(database::variables::table, -1)
