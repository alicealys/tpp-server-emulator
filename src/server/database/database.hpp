#pragma once

#include <utils/concurrency.hpp>

#pragma warning(push)
#pragma warning(disable: 4267)
#pragma warning(disable: 4018)
#pragma warning(disable: 4996)
#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/mysql/mysql.h>
#pragma warning(pop)

namespace sql = sqlpp::mysql;

namespace database
{
	using database_t = std::unique_ptr<sql::connection>;

	class table_interface
	{
	public:
		virtual void create(database_t& table)
		{
		}
	};

	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of<table_interface, T>::value, "table has invalid base class");

	public:
		installer()
		{
			register_table(std::make_unique<T>());
		}
	};

	void register_table(std::unique_ptr<table_interface>&& component);

	using tables = std::vector<std::unique_ptr<table_interface>>;
	tables& get_tables();

	database_t& get();

	std::string get_smart_device_id(const std::string& account_id);
}

#define REGISTER_TABLE(name)													\
namespace																		\
{																				\
	static database::installer<name> table__;									\
}																				\

#define DEFINE_FIELD(field_name, type)											\
struct field_name##_t															\
	{																			\
	struct _alias_t																\
	{																			\
		static constexpr const char _literal[] = #field_name;					\
		using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;	\
		template <typename T>													\
		struct _member_t														\
		{																		\
			T field_name;														\
			T& operator()()														\
			{																	\
				return field_name;												\
			}																	\
			const T& operator()() const											\
			{																	\
				return field_name;												\
			}																	\
		};																		\
	};																			\
																				\
	using _traits = sqlpp::make_traits<type>;									\
};																				\

#define DEFINE_TABLE(table_name, ...)											\
struct table_name##_t : sqlpp::table_t<table_name##_t, ##__VA_ARGS__>			\
{																				\
	struct _alias_t																\
	{																			\
		static constexpr const char _literal[] = #table_name;					\
		using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;	\
		template <typename T>													\
		struct _member_t														\
		{																		\
			T table_name;														\
			T& operator()()														\
			{																	\
				return table_name;												\
			}																	\
			const T& operator()() const											\
			{																	\
				return table_name;												\
			}																	\
		};																		\
	};																			\
};																				\

