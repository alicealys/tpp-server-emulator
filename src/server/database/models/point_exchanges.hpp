#pragma once

#include "../database.hpp"

namespace database::point_exchanges
{
	struct point_exchange_param_t
	{
		std::uint32_t common_value;
		std::uint16_t count;
		std::uint16_t exchange_limit;
		std::uint32_t info_lang_id;
		std::uint16_t limited_count;
		std::uint32_t name_lang_id;
		std::uint32_t point;
		std::uint8_t type;
		std::uint32_t unique_id;

		bool parse(nlohmann::json& data);
		void to_json(nlohmann::json& data) const;
	};

	class point_exchange_params_t : public std::vector<point_exchange_param_t>
	{
	public:
		bool parse(nlohmann::json& data);
		const std::vector<point_exchange_param_t>::const_iterator find(const point_exchange_param_t& param) const;
	};

	std::uint16_t get_exchange_limit(point_exchange_param_t& param, const point_exchange_params_t& exchanged);

	enum exchange_mode_t : std::uint32_t
	{
		exchange_mode_league = 0,
		exchange_mode_event = 1,
	};

	enum exchange_type_t : std::uint8_t
	{
		exchange_type_staff = 1,
		exchange_type_resource = 2,
		exchange_type_blueprint = 3,
		exchange_type_uniform = 4,
		exchange_type_weapon_color = 5,
		exchange_type_emblem = 6,
		exchange_type_nameplate = 8,
		exchange_type_coin = 9,
	};

	class point_exchange
	{
	public:
		DEFINE_FIELD(exchange_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(exchange_mode, sqlpp::integer_unsigned);
		DEFINE_FIELD(exchange_common_value, sqlpp::integer_unsigned);
		DEFINE_FIELD(exchange_type, sqlpp::integer_unsigned);
		DEFINE_FIELD(exchange_unique_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(expire_date, sqlpp::time_point);
		DEFINE_TABLE(point_exchanges, 
			exchange_id_field_t,
			player_id_field_t,
			exchange_mode_field_t,
			exchange_common_value_field_t,
			exchange_type_field_t,
			exchange_unique_id_field_t,
			expire_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		point_exchange(const sqlpp::result_row_t<Args...>& row)
		{
			this->exchange_id_ = row.exchange_id;
			this->player_id_ = row.player_id;
			this->exchange_mode_ = row.exchange_mode;
			this->exchange_common_value_ = row.exchange_common_value;
			this->exchange_type_ = row.exchange_type;
			this->exchange_unique_id_ = row.exchange_unique_id;
			this->expire_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.expire_date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, exchange_id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, exchange_mode);
		GET_FIELD_H(std::uint32_t, exchange_common_value);
		GET_FIELD_H(std::uint8_t, exchange_type);
		GET_FIELD_H(std::uint32_t, exchange_unique_id);
		GET_FIELD_H(std::chrono::seconds, expire_date);

	};

	bool add_entry(const std::uint64_t player_id, const std::uint32_t mode, const point_exchange_param_t& param, const std::chrono::seconds expire_date);
	point_exchange_params_t get_exchanged_params(const std::uint64_t player_id, const std::uint32_t mode);

	void delete_player_data(const std::uint64_t player_id);
}
