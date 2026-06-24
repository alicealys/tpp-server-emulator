#pragma once

#include "../database.hpp"

namespace database::mgo_item_purchases
{
	enum purchase_id_t : std::uint32_t
	{
		character_slot = 3001001
	};

	struct purchase_info_t
	{
		std::uint32_t purchase_id;
		std::uint32_t purchase_type;
		std::uint32_t category;
		std::uint32_t price;
	};

	std::unordered_map<std::uint32_t, purchase_info_t>& get_purchasable_item_map();

	std::optional<purchase_info_t> get_purchase_info(const std::uint32_t purchase_id);

	class mgo_item_purchase
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(purchase_id, sqlpp::integer_unsigned);
		DEFINE_TABLE(mgo_item_purchases, id_field_t, player_id_field_t, purchase_id_field_t);

		inline static table_t table;

		template <typename ...Args>
		mgo_item_purchase(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->purchase_id_ = row.purchase_id;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, purchase_id);

	};

	bool purchase_item(const std::uint64_t player_id, const std::uint32_t purchase_id);
	std::vector<std::uint32_t> get_purchase_list(const std::uint64_t player_id);
}
