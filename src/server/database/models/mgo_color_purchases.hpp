#pragma once

#include "../database.hpp"

namespace database::mgo_color_purchases
{
	enum item_category
	{
		none = 0,
		gear = 1,
		weapon = 2,
	};

	struct item_info_t
	{
		item_category category;
		std::uint32_t id;
		std::uint32_t default_color;
		std::uint32_t purchase_type;
		std::uint32_t point;
		std::uint32_t prestige;
		bool is_default;
	};

	struct color_t
	{
		std::uint32_t color;
		std::uint32_t point;
		std::uint32_t level;
		std::uint32_t prestige;
		std::uint32_t purchase_type;
	};

	item_info_t get_gear_info(const std::uint32_t gear_id);
	std::vector<color_t>& get_gear_colors();
	std::optional<color_t> get_gear_color(const std::uint32_t color_id);

	std::vector<color_t>& get_weapon_colors();
	std::optional<color_t> get_weapon_color(const std::uint32_t color_id);

	class mgo_color_purchase
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(category, sqlpp::integer_unsigned);
		DEFINE_FIELD(item_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(color_id, sqlpp::integer_unsigned);
		DEFINE_TABLE(mgo_color_purchases, id_field_t, category_field_t, player_id_field_t, item_id_field_t, color_id_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_color_purchase(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->category_ = row.category;
			this->item_id_ = row.item_id;
			this->color_id_ = row.color_id;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, category);
		GET_FIELD_H(std::uint32_t, item_id);
		GET_FIELD_H(std::uint32_t, color_id);

	};

	bool has_gear(const std::uint64_t player_id, const std::uint32_t gear_id);
	bool buy_color(const std::uint64_t player_id, const item_category category, const std::uint32_t item_id, const std::uint32_t color_id);
	std::unordered_set<std::uint32_t> get_purchased_colors(const std::uint64_t player_id, const item_category category, const std::uint32_t item_id);
	std::vector<mgo_color_purchase> get_all_purchased_colors(const std::uint64_t player_id, const item_category category);
	void delete_player_data(const std::uint64_t player_id);
}
