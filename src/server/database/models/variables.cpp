#include <std_include.hpp>

#include "variables.hpp"

namespace database::variables
{
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
	}

	void set(const std::string& name, const nlohmann::json& value)
	{
		RUN_IMPL(impl::set, name, value);
	}

	std::optional<nlohmann::json> get(const std::string& name)
	{
		RUN_IMPL(impl::get, name);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.variables.create");
		}
	};
}

REGISTER_TABLE(database::variables::table, -1)
