#pragma once

#include <utils/concurrency.hpp>

#pragma warning(push)
#pragma warning(disable: 4127)
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
		virtual void create(database_t& db)
		{
		}

		virtual void run_tasks(database_t& db)
		{
		}
	};

	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of<table_interface, T>::value, "table has invalid base class");

	public:
		installer(int priority)
		{
			register_table(std::make_unique<T>(), priority);
		}
	};

	void register_table(std::unique_ptr<table_interface>&& component, int priority);

	struct table_def
	{
		int priority;
		std::unique_ptr<table_interface> inst;
	};

	using tables = std::vector<table_def>;
	tables& get_tables();
}

#define REGISTER_TABLE(name, ...)												\
namespace																		\
{																				\
	static database::installer<name> table__(__VA_ARGS__);						\
}																				\

#define DEFINE_FIELD(field_name, type)											\
struct field_name##_field_t														\
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
struct table_t : sqlpp::table_t<table_t, ##__VA_ARGS__>\
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

#define GET_FIELD_H(__type__, __name__) \
public: \
	__type__ get_##__name__() const; \
private: \
	__type__ __name__##_; \
public: \

#define GET_FIELD_C(__class__, __type__, __name__) \
	__type__ __class__::get_##__name__() const \
	{ \
		return this->##__name__##_; \
	} \
