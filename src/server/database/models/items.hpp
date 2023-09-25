#pragma once

#include "../database.hpp"

#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::items
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(item_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(create_date, sqlpp::time_point);
	DEFINE_TABLE(items, id_field_t, player_id_field_t, item_id_field_t, create_date_field_t);

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
		std::uint32_t use_resource_values[2];
		std::uint32_t use_resource_ids[2];
	};

#ifdef DEBUG
	constexpr auto dev_limit = 999;
#else
	constexpr auto dev_limit = 4;
#endif

	nlohmann::json get_static_list_json();
	std::vector<item_t>& get_static_list();
	std::unordered_map<std::uint32_t, item_t>& get_static_map();
	item_t get_item_data(const std::uint32_t id);

	class item_status
	{
	public:
		template <typename ...Args>
		item_status(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = static_cast<std::uint32_t>(row.item_id);
			this->player_id_ = row.player_id;
			this->create_date_ = std::chrono::time_point<std::chrono::system_clock>(row.create_date.value().time_since_epoch());
			this->created_ = true;
			this->valid_ = true;

#ifdef DEBUG
			this->develop_ = 2;
			this->open_ = 1;
			this->left_second_ = 0;
#endif
		}

		item_status(const std::uint32_t item_id, const std::uint64_t player_id)
		{
			this->id_ = item_id;
			this->player_id_ = player_id;
			this->created_ = false;
			this->valid_ = true;
		}

		item_status()
		{
			this->valid_ = false;
		}

		void set_data(const item_t& item_data, std::unique_ptr<player_data::player_data>& p_data);

		int is_open() const
		{
			return this->open_;
		}

		std::uint32_t get_id() const
		{
			return this->id_;
		}

		std::uint32_t get_gmp() const
		{
			return this->gmp_;
		}

		std::uint32_t get_develop() const
		{
			return this->develop_;
		}

		std::uint32_t get_mb_coin() const
		{
			return this->mb_coin_;
		}

		std::uint32_t get_left_second() const
		{
			return this->left_second_;
		}

		std::uint32_t get_max_second() const
		{
			return this->max_second_;
		}

		std::chrono::system_clock::time_point get_create_date() const
		{
			return this->create_date_;
		}

		std::uint32_t get_create_time() const
		{
			if (!this->created_)
			{
				return 0;
			}

			return static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(this->create_date_.time_since_epoch()).count());
		}

		bool is_created() const
		{
			return this->created_;
		}

		std::uint32_t get_resource_id(const std::uint32_t index) const
		{
			return this->resource_ids_[index];
		}

		std::uint32_t get_resource_value(const std::uint32_t index) const
		{
			return this->resource_values_[index];
		}

	private:
		bool open_;
		bool created_;
		bool valid_;
		std::uint64_t player_id_;
		std::uint32_t id_;
		std::uint32_t gmp_;
		std::uint32_t develop_;
		std::uint32_t mb_coin_;
		std::uint32_t left_second_;
		std::uint32_t max_second_;
		std::uint32_t resource_ids_[2];
		std::uint32_t resource_values_[2];
		std::chrono::system_clock::time_point create_date_;

	};

	std::unordered_map<std::uint32_t, item_status> get_item_list(const std::uint64_t player_id);
	item_status get_item(const std::uint64_t player_id, const std::uint32_t item_id);
	void create(const std::uint64_t player_id, const std::uint32_t item_id);
}
