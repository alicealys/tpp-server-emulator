#pragma once

#include "../database.hpp"

#include "utils/tpp.hpp"

namespace database::mgo_stats
{
	std::vector<std::uint32_t> get_id_list();
	std::unordered_set<std::uint32_t> get_id_set();

	class mgo_stat
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(stat_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(stat_value, sqlpp::integer_unsigned);
		DEFINE_TABLE(mgo_stats, id_field_t, player_id_field_t, stat_id_field_t, stat_value_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_stat(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->stat_id_ = row.stat_id;
			this->stat_value_ = row.stat_value;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, stat_id);
		GET_FIELD_H(std::uint32_t, stat_value);

	};

	std::uint32_t get_stat(const std::uint64_t player_id, const std::uint32_t stat_id);
	bool set_stat(const std::uint64_t player_id, const std::uint32_t stat_id, const std::uint32_t stat_value);

	std::vector<mgo_stat> get_stats(const std::uint64_t player_id);
}
