#pragma once

#include "../database.hpp"

namespace database::mgo_titles
{
	struct mgo_title_def_t
	{
		std::uint32_t title_id;
		std::uint32_t gp;
	};

	const std::vector<mgo_title_def_t>& get_titles();

	class mgo_title
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(title_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(gp, sqlpp::integer_unsigned);
		DEFINE_FIELD(flag, sqlpp::integer_unsigned);
		DEFINE_FIELD(date, sqlpp::time_point);
		DEFINE_TABLE(mgo_titles, id_field_t, player_id_field_t, title_id_field_t, flag_field_t, gp_field_t, date_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_title(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->title_id_ = static_cast<std::uint32_t>(row.title_id);
			this->flag_ = static_cast<std::uint32_t>(row.flag);
			this->date_ = std::chrono::duration_cast<std::chrono::seconds>(row.date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, title_id);
		GET_FIELD_H(std::uint32_t, flag);
		GET_FIELD_H(std::uint32_t, gp);
		GET_FIELD_H(std::chrono::seconds, date);

	};

	void set_player_title(const std::uint64_t player_id, const std::uint32_t title_id, const std::uint32_t flag, const std::uint32_t gp);
	std::vector<mgo_title> get_player_title_list(const std::uint64_t player_id);
	std::unordered_map<std::uint32_t, mgo_title> get_player_title_map(const std::uint64_t player_id);
	void delete_player_data(const std::uint64_t player_id);
}
