#pragma once

#include "../database.hpp"

namespace database::shop_purchases
{
	enum item_type_t : std::uint32_t
	{
		time_reduction_dispatch = 10001,
		time_reduction_development = 10002,
		time_reduction_construction = 10003,
		time_reduction_development2 = 10009,
		insurance_7_days = 10010,
		insurance_30_days = 10011,
		insurance_60_days = 10012,
		en_10013 = 10013,
		en_10014 = 10014,
		en_10015 = 10015,
		insurance_7_days_free = 10016,
		waters_mid_atlantic_ridge = 10020,
		waters_east_of_the_hawaii_islands = 10021,
		waters_south_atlantic_ocean = 10022,
		waters_indian_ocean = 10023,
		waters_north_pacific_ocean = 10024,
		waters_south_pacific_ocean = 10025,
		waters_north_atlantic_ocean = 10026,
		waters_north_pacific_ocean_free = 10027,
		waters_south_pacific_ocean_free = 10028,
		waters_north_atlantic_ocean_free = 10029,
		waters_transfer_fee = 10030,
		waters_north_pacific_ocean_free2 = 20001,
		waters_south_pacific_ocean_free2 = 20002,
		waters_north_atlantic_ocean_free2 = 20003,

		fob_defense_insurance_beg = 12000,
		fob_defense_insurance_1_day = 12010,
		fob_defense_insurance_3_day = 12011,
		fob_defense_insurance_7_day = 12012,
		fob_defense_insurance_14_day = 12013,
		fob_defense_insurance_3_day_free = 12022,
		fob_defense_insurance_end = 13000,

		fob_unit_deployment = 21001,
		weapons_color_variation = 1000000,
		gears_color_variation = 2000000,
		character_slot = 3000000,
		gp_boost = 4000000,
		adjustment_due_to_free_mb_coin_bug = 9000000,
	};

	struct shop_item_entry_t
	{
		std::uint32_t item_type;
		std::array<std::string, 20> name;
		std::uint32_t price;
	};

	std::vector<shop_item_entry_t>& get_shop_item_list();

	std::uint32_t get_waters_item_type(const std::uint32_t area_id);

	struct entry_params_t
	{
		std::uint32_t event_type;
		std::uint32_t item_quantity;
		std::uint32_t item_type;
		std::uint32_t param1;
		std::uint32_t coin_quantity;
	};

	class shop_purchase
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(date, sqlpp::time_point);
		DEFINE_FIELD(expire_date, sqlpp::time_point);
		DEFINE_FIELD(event_type, sqlpp::integer_unsigned);
		DEFINE_FIELD(item_quantity, sqlpp::integer_unsigned);
		DEFINE_FIELD(item_type, sqlpp::integer_unsigned);
		DEFINE_FIELD(param1, sqlpp::integer_unsigned);
		DEFINE_FIELD(remaining_coin, sqlpp::integer_unsigned);
		DEFINE_FIELD(coin_quantity, sqlpp::integer_unsigned);
		DEFINE_TABLE(shop_purchases, id_field_t, player_id_field_t, date_field_t, expire_date_field_t, event_type_field_t,
			item_quantity_field_t, item_type_field_t, param1_field_t, remaining_coin_field_t, coin_quantity_field_t);

		inline static table_t table;

		template <typename ...Args>
		shop_purchase(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->date_ = std::chrono::duration_cast<std::chrono::seconds>(row.date.value().time_since_epoch());
			this->expire_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.expire_date.value().time_since_epoch());
			this->event_type_ = static_cast<std::uint32_t>(row.event_type);
			this->item_quantity_ = static_cast<std::uint32_t>(row.item_quantity);
			this->item_type_ = static_cast<std::uint32_t>(row.item_type);
			this->param1_ = static_cast<std::uint32_t>(row.param1);
			this->remaining_coin_ = static_cast<std::uint32_t>(row.remaining_coin);
			this->coin_quantity_ = static_cast<std::uint32_t>(row.coin_quantity);
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::chrono::seconds, date);
		GET_FIELD_H(std::chrono::seconds, expire_date);
		GET_FIELD_H(std::uint32_t, event_type);
		GET_FIELD_H(std::uint32_t, item_quantity);
		GET_FIELD_H(std::uint32_t, item_type);
		GET_FIELD_H(std::uint32_t, param1);
		GET_FIELD_H(std::uint32_t, remaining_coin);
		GET_FIELD_H(std::uint32_t, coin_quantity);

	};

	bool add_entry(const std::uint64_t player_id, const entry_params_t& params);
	bool add_spent_single(const std::uint64_t player_id, const std::uint32_t item_type, const std::uint32_t price, const std::uint32_t param1 = 0u);
	bool add_spent(const std::uint64_t player_id, const std::uint32_t item_type, const std::uint32_t quantity, const std::uint32_t price,
		const std::uint32_t param1);

	std::size_t get_history_size(const std::uint64_t player_id);
	std::vector<shop_purchase> get_history(const std::uint64_t player_id, const std::uint32_t limit);
	std::optional<shop_purchase> get_last_item_purchase(const std::uint64_t player_id, const std::uint32_t item_type);
	std::optional<shop_purchase> get_last_item_purchase_range(const std::uint64_t player_id, const std::uint32_t item_type_beg, const std::uint32_t item_type_end);
	void delete_player_data(const std::uint64_t player_id);
}
