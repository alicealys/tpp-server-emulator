#pragma once

#include "../database.hpp"

namespace database::mgo_color_purchase
{
	class mgo_color_purchase
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(gear_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(color_id, sqlpp::integer_unsigned);
		DEFINE_TABLE(mgo_color_purchase, id_field_t, player_id_field_t, gear_id_field_t, color_id_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_color_purchase(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->gear_id_ = row.gear_id;
			this->color_id_ = row.color_id;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, gear_id);
		GET_FIELD_H(std::uint32_t, color_id);

	};

	bool has_gear(const std::uint64_t player_id, const std::uint32_t gear_id);
	bool buy_color(const std::uint64_t player_id, const std::uint32_t gear_id, const std::uint32_t color_id);
	std::unordered_set<std::uint32_t> get_purchased_colors(const std::uint64_t player_id, const std::uint32_t gear_id);
}
