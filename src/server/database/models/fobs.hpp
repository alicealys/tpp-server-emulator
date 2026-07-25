#pragma once

#include "../database.hpp"

#include "player_data.hpp"

namespace database::fobs
{
	nlohmann::json& get_area_list();
	std::optional<nlohmann::json> get_area(const std::uint32_t area_id);

	void apply_deploy_damage_params(const std::uint64_t fob_id, game::fob_cluster_param_t& cluster_param, std::optional<nlohmann::json>& deploy_damage);

	constexpr auto fob_id_reserve_count = 1000ull;

	class fob
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(area_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(platform_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(security_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(construct_param, sqlpp::integer_unsigned);
		DEFINE_FIELD(cluster_param, sqlpp::binary);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(fobs, id_field_t, player_id_field_t, fob_index_field_t,
			area_id_field_t, platform_count_field_t, security_rank_field_t,
			cluster_param_field_t, construct_param_field_t, create_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		fob(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->index_ = row.fob_index;

			this->param_.area_id = static_cast<std::uint16_t>(row.area_id);
			this->param_.construct_param.packed = static_cast<std::uint32_t>(row.construct_param);
			this->param_.platform_count = static_cast<std::uint8_t>(row.platform_count);
			this->param_.security_rank = static_cast<std::uint8_t>(row.security_rank);

			this->create_date_ = row.create_date.value().time_since_epoch();

			load_binary_field(&this->param_.cluster_param, row.cluster_param.value());
		}

		std::uint64_t get_id() const
		{
			return this->id_;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint64_t get_index() const
		{
			return this->index_;
		}

		std::uint32_t get_area_id() const
		{
			return this->param_.area_id;
		}

		std::uint32_t get_platform_count() const
		{
			return this->param_.platform_count;
		}

		std::uint32_t get_security_rank() const
		{
			return this->param_.security_rank;
		}

		game::fob_construct_param_t get_construct_param() const
		{
			return this->param_.construct_param;
		}

		const game::fob_cluster_param_t& get_cluster_param() const
		{
			return this->param_.cluster_param;
		}

		game::fob_cluster_param_t& get_cluster_param()
		{
			return this->param_.cluster_param;
		}

		std::chrono::microseconds get_creation_time() const
		{
			return this->create_date_;
		}

	private:
		std::uint64_t id_{};
		std::uint64_t player_id_{};
		std::uint64_t index_{};
		game::fob_param_t param_{};
		std::chrono::microseconds create_date_;

	};

	std::optional<fob> get_fob(const std::uint64_t id);
	std::vector<fob> get_fob_list(const std::uint64_t player_id);

	void create(const std::uint64_t player_id, const std::uint32_t area_id, const std::uint64_t fob_id = 0);

	void sync_data(const std::uint64_t player_id, std::vector<game::fob_param_t>& fob_params);
	void set_construct_param(const std::uint64_t player_id, const std::uint64_t fob_index, const game::fob_construct_param_t& param);

	void delete_all(const std::uint64_t player_id);
}
