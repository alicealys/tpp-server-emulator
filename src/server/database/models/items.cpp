#include <std_include.hpp>

#include "items.hpp"

#include "game/game.hpp"

namespace database::items
{
	namespace
	{
		std::vector<item_t> load_static_list()
		{
			static const auto data = database::items::get_static_list_json();
			std::vector<item_t> items;

			if (!data.is_array())
			{
				return items;
			}

			for (auto i = 0ull; i < data.size(); i++)
			{
				item_t item{};
				auto& entry = data[i];

				item.dev_coin = entry["dev_coin"].get<std::uint32_t>();
				item.dev_gmp = entry["dev_gmp"].get<std::int32_t>();
				item.dev_item_1 = entry["dev_item_1"].get<std::uint32_t>();
				item.dev_item_2 = entry["dev_item_2"].get<std::uint32_t>();

				for (auto plat = 0; plat < 7; plat++)
				{
					const auto key = std::format("dev_platlv0{}", plat + 1);
					item.dev_platform_levels[plat] = entry[key].get<std::uint32_t>();
				}

				item.dev_resource_values[0] = entry["dev_rescount01_value"].get<std::uint32_t>();
				item.dev_resource_values[1] = entry["dev_rescount02_value"].get<std::uint32_t>();

				item.dev_resource_ids[0] = entry["dev_resource01_id"].get<std::uint32_t>();
				item.dev_resource_ids[1] = entry["dev_resource02_id"].get<std::uint32_t>();

				item.dev_skill = entry["dev_skil"].get<std::uint32_t>();
				item.dev_special = entry["dev_special"].get<std::uint32_t>();
				item.dev_time = entry["dev_time"].get<std::uint32_t>();

				item.id = entry["id"].get<std::uint32_t>();
				item.type = entry["type"].get<std::uint32_t>();
				item.open = entry["open"].get<std::uint32_t>();

				item.use_gmp = entry["use_gmp"].get<std::uint32_t>();

				item.use_resource_values[0] = entry["use_rescount01_value"].get<std::uint32_t>();
				item.use_resource_values[1] = entry["use_rescount02_value"].get<std::uint32_t>();

				item.use_resource_ids[0] = entry["use_resource01_id"].get<std::uint32_t>();
				item.use_resource_ids[1] = entry["use_resource02_id"].get<std::uint32_t>();

				items.emplace_back(item);
			}

			return items;
		}

		std::unordered_map<std::uint32_t, item_t> load_static_map()
		{
			static const auto list = load_static_list();
			std::unordered_map<std::uint32_t, item_t> map;

			for (const auto& item : list)
			{
				map.insert(std::make_pair(item.id, item));
			}

			return map;
		}
	}

	nlohmann::json get_static_list_json()
	{
		static const auto data = utils::resources::load_json(RESOURCE_ITEM_LIST);
		return data;
	}

	std::vector<item_t>& get_static_list()
	{
		static std::vector<item_t> list = load_static_list();
		return list;
	}

	std::unordered_map<std::uint32_t, item_t>& get_static_map()
	{
		static std::unordered_map<std::uint32_t, item_t> list = load_static_map();
		return list;
	}

	item_t get_item_data(const std::uint32_t id)
	{
		const auto& map = get_static_map();
		const auto iter = map.find(id);
		if (iter == map.end())
		{
			return {};
		}

		return iter->second;
	}

	bool item_status::can_develop(const std::optional<player_data::player_data>& player_data,
		const database::player_data::resource_arrays_t& resource_arrays, const database::player_data::staff_array_container& staff_array) const
	{
		const auto count_1 = resource_arrays[game::processed_server][this->resource_ids_[0]];
		const auto count_2 = resource_arrays[game::processed_server][this->resource_ids_[1]];
		const auto sever_gmp = player_data->get_server_gmp();

		if (sever_gmp < static_cast<std::int32_t>(this->gmp_) ||
			count_1 < this->resource_values_[0] ||
			count_2 < this->resource_values_[1])
		{
			return false;
		}

		return true;
	}

	GET_FIELD_C(item_status, std::uint64_t, item_develop_id);
	GET_FIELD_C(item_status, std::uint64_t, player_id);
	GET_FIELD_C(item_status, std::uint32_t, item_type);
	GET_FIELD_C(item_status, std::uint32_t, state);
	GET_FIELD_C(item_status, std::chrono::seconds, create_date);

	item_status::item_status()
	{
	}

	item_status::item_status(const std::uint32_t item_type, const std::uint64_t player_id)
	{
		this->item_type_ = item_type;
		this->player_id_ = player_id;
		this->created_ = false;
		this->initialize();
	}

	void item_status::initialize()
	{
		if (this->item_type_ == 0)
		{
			return;
		}

		const auto& static_map = get_static_map();
		const auto iter = static_map.find(this->item_type_);

		if (iter == static_map.end())
		{
			return;
		}

		const auto& item_data = iter->second;
		const auto now = std::chrono::system_clock::now();
		const auto now_s = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

		this->max_second_ = item_data.dev_time * 60;
		this->gmp_ = item_data.dev_gmp;
		this->skill_ = item_data.dev_skill;
		std::memcpy(&this->platform_levels_, item_data.dev_platform_levels, sizeof(item_data.dev_platform_levels));

		this->resource_ids_[0] = item_data.dev_resource_ids[0];
		this->resource_ids_[1] = item_data.dev_resource_ids[1];
		this->resource_values_[0] = item_data.dev_resource_values[0];
		this->resource_values_[1] = item_data.dev_resource_values[1];

		if (this->created_)
		{
			const auto seconds_passed = (now_s - this->create_date_).count();
			if (static_cast<std::int64_t>(this->max_second_) < seconds_passed)
			{
				this->left_second_ = 0;
				this->develop_ = 2;
			}
			else
			{
				this->left_second_ = static_cast<std::uint32_t>(static_cast<std::uint64_t>(this->max_second_) - seconds_passed);
				this->develop_ = 1;
			}
		}
		else
		{
			this->left_second_ = this->max_second_;
			this->develop_ = 0;
		}

		this->mb_coin_ = game::calculate_mb_coins(this->left_second_, database::vars.cost_factor_item_dev);
		this->open_ = item_data.open != 0 || this->state_ == item_state_develop || this->state_ == item_state_open;

		if (vars.unlock_all_items && this->item_type_ != item_id::nuclear)
		{
			this->develop_ = 2;
			this->open_ = 1;
			this->left_second_ = 0;
		}
	}

	std::uint32_t item_status::get_gmp() const
	{
		return this->gmp_;
	}

	std::uint32_t item_status::get_develop() const
	{
		return this->develop_;
	}

	std::uint32_t item_status::get_mb_coin() const
	{
		return this->mb_coin_;
	}

	std::uint32_t item_status::get_left_second() const
	{
		return this->left_second_;
	}

	std::uint32_t item_status::get_max_second() const
	{
		return this->max_second_;
	}

	bool item_status::is_open() const
	{
		return this->open_;
	}

	bool item_status::is_created() const
	{
		return this->created_;
	}

	std::uint32_t item_status::get_resource_id(const std::uint32_t index) const
	{
		return this->resource_ids_[index];
	}

	std::uint32_t item_status::get_resource_value(const std::uint32_t index) const
	{
		return this->resource_values_[index];
	}

	void item_status::to_json(nlohmann::json& data) const
	{
		data["create_date"] = this->create_date_.count();
		data["develop"] = this->develop_;
		data["gmp"] = this->gmp_;
		data["id"] = this->item_type_;
		data["left_second"] = this->left_second_;
		data["max_second"] = this->max_second_;
		data["mb_coin"] = this->mb_coin_;
		data["open"] = std::uint8_t(this->open_);
	}

	std::vector<item_status>::iterator item_list::find(const std::uint32_t item_type)
	{
		const auto iter = std::lower_bound(this->begin(), this->end(), item_type, [](const item_status& status, const std::uint32_t value)
		{
			return status.item_type_ < value;
		});

		if (iter == this->end() || iter->item_type_ != item_type)
		{
			return this->end();
		}

		return iter;
	}

	const std::vector<item_status>::const_iterator item_list::find(const std::uint32_t item_type) const
	{
		const auto iter = std::lower_bound(this->begin(), this->end(), item_type, [](const item_status& status, const std::uint32_t value)
		{
			return status.item_type_ < value;
		});

		if (iter == this->end() || iter->item_type_ != item_type)
		{
			return this->end();
		}

		return iter;
	}

	namespace impl
	{
		template <database_type_t Type>
		item_list get_item_list(const std::uint64_t player_id, bool merge_list)
		{
			item_list list;

			database::access([&](database::database_t& db)
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(item_status::table))
							.from(item_status::table)
								.where(item_status::table.player_id == player_id)
									.order_by(item_status::table.item_id.asc()));

				for (auto& row : results)
				{
					list.emplace_back(row);
				}
			});

			if (!merge_list)
			{
				return list;
			}

			item_list merged_list;

			const auto& static_list = get_static_list();
			for (const auto& item : static_list)
			{
				const auto iter = list.find(item.id);
				if (iter == list.end())
				{
					merged_list.emplace_back(item.id, player_id);
				}
				else
				{
					merged_list.emplace_back(*iter);
				}
			}

			std::sort(merged_list.begin(), merged_list.end(), [](const item_status& a, const item_status& b)
			{
				return a.get_item_type() < b.get_item_type();
			});

			return merged_list;
		}

		template <database_type_t Type>
		item_status get_item(const std::uint64_t player_id, const std::uint32_t item_id)
		{
			auto status = database::access<item_status>([&](database::database_t& db)
				-> item_status
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(item_status::table))
							.from(item_status::table)
								.where(item_status::table.player_id == player_id &&
									   item_status::table.item_id == item_id));

				if (results.empty())
				{
					return item_status(item_id, player_id);
				}

				return item_status(results.front());
			});

			status.initialize();
			return status;
		}

		template <database_type_t Type>
		bool create(const std::uint64_t player_id, const std::uint32_t item_id, const std::uint32_t item_state)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::insert_into(item_status::table)
						.set(item_status::table.player_id = player_id,
							 item_status::table.item_id = item_id,
							 item_status::table.item_state = item_state,
							 item_status::table.create_date = std::chrono::system_clock::now()
					));

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool create_existing(const std::uint64_t item_develop_id, const std::uint32_t item_state)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(item_status::table)
						.set(item_status::table.item_state = item_state,
							 item_status::table.create_date = std::chrono::system_clock::now())
							.where(item_status::table.id == item_develop_id));

				return result != 0;
			});
		}
	
		template <database_type_t Type>
		bool remove(const std::uint64_t player_id, const std::uint32_t item_id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::remove_from(item_status::table)
						.where(item_status::table.player_id == player_id &&
							   item_status::table.item_id == item_id)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		bool force_develop(const std::uint64_t item_develop_id)
		{
			return database::access<bool>([&](database::database_t& db)
			{
				const auto result = db.get_database<Type>()->operator()(
					sqlpp::update(item_status::table)
						.set(item_status::table.create_date = std::chrono::system_clock::from_time_t({}))
							.where(item_status::table.id == item_develop_id)
					);

				return result != 0;
			});
		}

		template <database_type_t Type>
		void delete_player_data(const std::uint64_t player_id)
		{
			database::access([&](database_t& db)
			{
				db.get_database<Type>()->operator()(
					sqlpp::remove_from(item_status::table)
						.where(item_status::table.player_id == player_id));
			});
		}
	}

	item_list get_item_list(const std::uint64_t player_id, bool merge_list)
	{
		RUN_IMPL(impl::get_item_list, player_id, merge_list);
	}

	item_status get_item(const std::uint64_t player_id, const std::uint32_t item_id)
	{
		RUN_IMPL(impl::get_item, player_id, item_id);
	}

	bool create(const std::uint64_t player_id, const std::uint32_t item_id, const std::uint32_t item_state)
	{
		RUN_IMPL(impl::create, player_id, item_id, item_state);
	}

	bool create_existing(const std::uint64_t item_develop_id, const std::uint32_t item_state)
	{
		RUN_IMPL(impl::create_existing, item_develop_id, item_state);
	}

	bool remove(const std::uint64_t player_id, const std::uint32_t item_id)
	{
		RUN_IMPL(impl::remove, player_id, item_id);
	}

	bool force_develop(const std::uint64_t item_develop_id)
	{
		RUN_IMPL(impl::force_develop, item_develop_id);
	}

	void delete_player_data(const std::uint64_t player_id)
	{
		RUN_IMPL(impl::delete_player_data, player_id);
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.items.create");

			get_static_map();
			get_static_list();
			get_static_list_json();
		}
	};
}

REGISTER_TABLE(database::items::table, -1)
