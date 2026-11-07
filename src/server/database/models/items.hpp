#pragma once

#include "../database.hpp"

#include "player_data.hpp"

namespace database::items
{
	struct item_t
	{
		std::uint32_t dev_coin;
		std::int32_t dev_gmp;
		std::uint32_t dev_item_1;
		std::uint32_t dev_item_2;
		std::uint32_t dev_platform_levels[7];
		std::uint32_t dev_resource_values[2];
		std::uint32_t dev_resource_ids[2];
		std::uint32_t dev_skill;
		std::uint32_t dev_special;
		std::uint32_t dev_time;
		std::uint32_t id;
		std::uint32_t type;
		std::uint32_t use_gmp;
		std::uint32_t open;
		std::uint32_t use_resource_values[2];
		std::uint32_t use_resource_ids[2];
	};
	
	enum item_id
	{
		nuclear = 39001
	};

	enum develop_status_t
	{
		item_develop_undeveloped = 0,
		item_develop_indev = 1,
		item_develop_developed = 2
	};

	enum item_state_t
	{
		item_state_none = 0,
		item_state_open = 1,
		item_state_develop = 2,
	};

	nlohmann::json get_static_list_json();
	std::vector<item_t>& get_static_list();
	std::unordered_map<std::uint32_t, item_t>& get_static_map();
	item_t get_item_data(const std::uint32_t id);

	class item_status
	{
	public:
		friend class item_list;

		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(item_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(item_state, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(items, id_field_t, player_id_field_t, item_id_field_t, item_state_field_t, create_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		item_status(const sqlpp::result_row_t<Args...>& row)
		{
			this->item_develop_id_ = row.id;
			this->item_type_ = static_cast<std::uint32_t>(row.item_id);
			this->player_id_ = row.player_id;
			this->create_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.create_date.value().time_since_epoch());
			this->state_ = static_cast<std::uint32_t>(row.item_state);
			this->created_ = this->state_ == item_state_develop;
			this->initialize();
		}

		item_status();
		item_status(const std::uint32_t item_type, const std::uint64_t player_id);

		bool can_develop(const std::optional<player_data::player_data>& player_data,
			const database::player_data::resource_arrays_t& resource_arrays, 
			const database::player_data::staff_array_container& staff_array) const;
		void initialize();

		std::uint32_t get_gmp() const;
		std::uint32_t get_develop() const;
		std::uint32_t get_mb_coin() const;

		std::uint32_t get_left_second() const;
		std::uint32_t get_max_second() const;

		std::uint32_t get_resource_id(const std::uint32_t index) const;
		std::uint32_t get_resource_value(const std::uint32_t index) const;

		bool is_open() const;
		bool is_created() const;

		void to_json(nlohmann::json& data) const;

		GET_FIELD_H(std::uint64_t, item_develop_id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, item_type);
		GET_FIELD_H(std::uint32_t, state);
		GET_FIELD_H(std::chrono::seconds, create_date);

	private:
		bool open_{};
		bool created_{};

		std::uint32_t gmp_{};
		std::uint32_t develop_{};
		std::uint32_t mb_coin_{};
		std::uint32_t left_second_{};
		std::uint32_t max_second_{};
		std::uint32_t resource_ids_[2]{};
		std::uint32_t resource_values_[2]{};
		std::uint32_t skill_{};
		std::uint32_t platform_levels_[7]{};

	};

	class item_list : public std::vector<item_status>
	{
	public:
		std::vector<item_status>::iterator find(const std::uint32_t item_type);
		const std::vector<item_status>::const_iterator find(const std::uint32_t item_type) const;
	};

	item_list get_item_list(const std::uint64_t player_id, bool merge_list = true);
	item_status get_item(const std::uint64_t player_id, const std::uint32_t item_id);

	bool create(const std::uint64_t player_id, const std::uint32_t item_id, const std::uint32_t item_state);
	bool create_existing(const std::uint64_t item_develop_id, const std::uint32_t item_state);
	bool remove(const std::uint64_t player_id, const std::uint32_t item_id);
	bool force_develop(const std::uint64_t item_develop_id);

	void delete_player_data(const std::uint64_t player_id);
}
