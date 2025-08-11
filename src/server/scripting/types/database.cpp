#include <std_include.hpp>

#include "../engine.hpp"

#include "database/database.hpp"
#include "database/vars.hpp"

namespace tpp::scripting
{
	namespace
	{
#ifdef MYSQL_SUPPORTED
		namespace mysql
		{
			nlohmann::json field_to_value(const MYSQL_FIELD* field, const std::string& row)
			{
				switch (field->type)
				{
				case enum_field_types::MYSQL_TYPE_INT24:
				case enum_field_types::MYSQL_TYPE_LONG:
				case enum_field_types::MYSQL_TYPE_SHORT:
					return std::atoi(row.data());
				case enum_field_types::MYSQL_TYPE_LONGLONG:
					return row;
				case enum_field_types::MYSQL_TYPE_FLOAT:
				case enum_field_types::MYSQL_TYPE_DOUBLE:
					return static_cast<float>(std::atof(row.data()));
				}

				return row;
			}

			nlohmann::json bind_to_value(const MYSQL_BIND* bind)
			{
				switch (bind->buffer_type)
				{
				case enum_field_types::MYSQL_TYPE_INT24:
				case enum_field_types::MYSQL_TYPE_LONG:
				case enum_field_types::MYSQL_TYPE_SHORT:
					return *reinterpret_cast<int*>(bind->buffer);
				case enum_field_types::MYSQL_TYPE_LONGLONG:
					return std::to_string(*reinterpret_cast<std::int64_t*>(bind->buffer));
				case enum_field_types::MYSQL_TYPE_FLOAT:
					return *reinterpret_cast<float*>(bind->buffer);
				case enum_field_types::MYSQL_TYPE_DOUBLE:
					return static_cast<float>(*reinterpret_cast<double*>(bind->buffer));
				case enum_field_types::MYSQL_TYPE_STRING:
					return std::string{reinterpret_cast<char*>(bind->buffer), bind->buffer_length};
				}

				return {};
			}

			nlohmann::json generate_result(MYSQL_STMT* stmt)
			{
				if (stmt == nullptr)
				{
					return {};
				}

				utils::memory::allocator allocator;

				nlohmann::json result_arr;

				const auto meta = mysql_stmt_result_metadata(stmt);
				if (meta == nullptr)
				{
					return {};
				}

				const auto column_count = mysql_num_fields(meta);
				const auto fields = mysql_fetch_fields(meta);

				const auto is_null = allocator.allocate_array<sqlpp::mysql::my_bool>(column_count);
				const auto errors = allocator.allocate_array<sqlpp::mysql::my_bool>(column_count);
				const auto real_lengths = allocator.allocate_array<unsigned long>(column_count);
				const auto binds = allocator.allocate_array<MYSQL_BIND>(column_count);

				for (auto i = 0u; i < column_count; i++)
				{
					binds[i].length = &real_lengths[i];
					binds[i].is_null = &is_null[i];
					binds[i].error = &errors[i];
				}

				if (mysql_stmt_bind_result(stmt, binds) != 0 ||
					mysql_stmt_store_result(stmt) != 0)
				{
					return {};
				}

				while (mysql_stmt_fetch(stmt) != MYSQL_NO_DATA)
				{
					for (auto i = 0u; i < column_count; i++)
					{
						switch (fields[i].type)
						{
						case enum_field_types::MYSQL_TYPE_INT24:
						case enum_field_types::MYSQL_TYPE_LONG:
						case enum_field_types::MYSQL_TYPE_SHORT:
						{
							binds[i].buffer_type = MYSQL_TYPE_LONG;
							binds[i].buffer = allocator.allocate<int>();
							binds[i].buffer_length = sizeof(int);
							break;
						};
						case enum_field_types::MYSQL_TYPE_LONGLONG:
						{
							binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
							binds[i].buffer = allocator.allocate<std::int64_t>();
							binds[i].buffer_length = sizeof(std::int64_t);
							break;
						};
						case enum_field_types::MYSQL_TYPE_FLOAT:
							binds[i].buffer_type = MYSQL_TYPE_FLOAT;
							binds[i].buffer = allocator.allocate<float>();
							binds[i].buffer_length = sizeof(float);
							break;
						case enum_field_types::MYSQL_TYPE_DOUBLE:
						{
							binds[i].buffer_type = MYSQL_TYPE_DOUBLE;
							binds[i].buffer = allocator.allocate<double>();
							binds[i].buffer_length = sizeof(double);
							break;
						}
						default:
							binds[i].buffer_type = MYSQL_TYPE_STRING;
							binds[i].buffer = allocator.allocate_array<char>(real_lengths[i]);
							binds[i].buffer_length = real_lengths[i];
							break;
						}

						mysql_stmt_fetch_column(stmt, &binds[i], i, 0);
					}

					nlohmann::json row_arr;

					for (auto i = 0u; i < column_count; i++)
					{
						const auto field = &fields[i];
						const std::string field_name = {field->name, field->name_length};
						row_arr[field_name] = bind_to_value(&binds[i]);
					}

					result_arr.emplace_back(row_arr);
				}

				mysql_free_result(meta);

				return result_arr;
			}

			nlohmann::json prepared_statement(database::database_t& db, const std::string& query, const sol::variadic_args& values)
			{
				MYSQL_BIND* binds = nullptr;
				size_t bind_count = 0;

				const auto free_binds = [=]
				{
					if (binds == nullptr)
					{
						return;
					}

					for (auto i = 0u; i < bind_count; i++)
					{
						utils::memory::free(binds[i].buffer);
					}

					utils::memory::free(binds);
				};

				const auto _0 = gsl::finally([&]
				{
					free_binds();
				});

				bind_count = values.size();
				binds = utils::memory::allocate_array<MYSQL_BIND>(bind_count);

				for (auto i = 0u; i < values.size(); i++)
				{
					const auto& arg = values[i];

					if (arg.is<float>())
					{
						binds[i].buffer = utils::memory::allocate<float>();
						binds[i].buffer_type = MYSQL_TYPE_FLOAT;
						*reinterpret_cast<float*>(binds[i].buffer) = arg.get<float>();
					}
					else if (arg.is<int>())
					{
						binds[i].buffer = utils::memory::allocate<int>();
						binds[i].buffer_type = MYSQL_TYPE_LONG;
						*reinterpret_cast<int*>(binds[i].buffer) = arg.get<int>();
					}
					else if (arg.is<std::string>())
					{
						const auto str = arg.get<std::string>();
						const auto str_copy = utils::memory::duplicate_string(str);
						binds[i].buffer = str_copy;
						binds[i].buffer_length = static_cast<std::uint32_t>(str.size());
						binds[i].buffer_type = MYSQL_TYPE_STRING;
					}
					else
					{
						binds[i].buffer_type = MYSQL_TYPE_NULL;
					}
				}

				const auto handle = db.get_mysql()->get_handle();
				const auto stmt = mysql_stmt_init(handle);

				nlohmann::json result;

				result["result"] = {};

				if (mysql_stmt_prepare(stmt, query.data(), static_cast<std::uint32_t>(query.size())) != 0 ||
					mysql_stmt_bind_param(stmt, binds) != 0 ||
					mysql_stmt_execute(stmt) != 0)
				{
					result["error"] = mysql_stmt_error(stmt);
					return result;
				}

				result["affected_rows"] = static_cast<std::int32_t>(mysql_stmt_affected_rows(stmt));
				result["result"] = mysql::generate_result(stmt);

				return result;
			}
		}
#endif
		nlohmann::json prepared_statement(const std::string& query, const sol::variadic_args& values)
		{
			return database::access<nlohmann::json>([&](database::database_t& db)
				-> nlohmann::json
			{
#ifdef MYSQL_SUPPORTED
				if (database::get_database_type() == database::database_mysql)
				{
					return mysql::prepared_statement(db, query, values);
				}
#endif

				return {};
			});
		}
	}

	void engine::setup_database()
	{
		this->state_["database"] = sol::state::create_table(this->state_.lua_state());
		this->state_["database"]["vars"] = sol::state::create_table(this->state_.lua_state());

		this->state_["database"]["vars"]["session_heartbeat"] = database::vars.session_heartbeat.count();
		this->state_["database"]["vars"]["session_timeout"] = database::vars.session_timeout.count();
		this->state_["database"]["vars"]["nuclear_find_probability"] = database::vars.nuclear_find_probability;
		this->state_["database"]["vars"]["wormhole_duration"] = database::vars.wormhole_duration.count();
		this->state_["database"]["vars"]["max_server_gmp"] = database::vars.max_server_gmp;
		this->state_["database"]["vars"]["max_local_gmp"] = database::vars.max_local_gmp;
		this->state_["database"]["vars"]["gmp_ratio"] = database::vars.gmp_ratio;
		this->state_["database"]["vars"]["item_dev_limit"] = database::vars.item_dev_limit;
		this->state_["database"]["vars"]["unlock_all_items"] = database::vars.unlock_all_items;
		this->state_["database"]["vars"]["cost_factor_generic"] = database::vars.cost_factor_generic;
		this->state_["database"]["vars"]["cost_factor_item_dev"] = database::vars.cost_factor_item_dev;
		this->state_["database"]["vars"]["cost_factor_platform_construction"] = database::vars.cost_factor_platform_construction;

		this->state_["database"]["execute"] = prepared_statement;
	}
}
