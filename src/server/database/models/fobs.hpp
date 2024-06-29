#pragma once

#include "../database.hpp"

#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::fobs
{
	nlohmann::json& get_area_list();
	std::optional<nlohmann::json> get_area(const std::uint32_t area_id);

	class fob
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(area_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(platform_count, sqlpp::integer_unsigned);
		DEFINE_FIELD(security_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(cluster_param, sqlpp::text);
		DEFINE_FIELD(construct_param, sqlpp::integer_unsigned);
		DEFINE_FIELD(create_date, sqlpp::time_point);
		DEFINE_TABLE(fobs, id_field_t, player_id_field_t, fob_index_field_t,
			area_id_field_t, platform_count_field_t, security_rank_field_t,
			cluster_param_field_t, construct_param_field_t, create_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		fob(const sqlpp::result_row_t<Args...>& row)
		{
			try
			{
				this->cluster_param_ = nlohmann::json::parse(row.cluster_param.value());
			}
			catch (const std::exception& e)
			{
				printf("failed to parse cluster param: %s\n", e.what());
			}

			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->index_ = row.fob_index;
			this->area_id_ = static_cast<std::uint32_t>(row.area_id);
			this->construct_param_ = static_cast<std::uint32_t>(row.construct_param);
			this->create_date_ = row.create_date.value().time_since_epoch();
			this->platform_count_ = static_cast<std::uint32_t>(row.platform_count);
			this->security_rank_ = static_cast<std::uint32_t>(row.security_rank);
		}

		fob(const nlohmann::json& json)
		{
			this->platform_count_ = json["platform_count"].get<std::uint32_t>();
			this->security_rank_ = json["security_rank"].get<std::uint32_t>();
			this->construct_param_ = json["construct_param"].get<std::uint32_t>();
			this->cluster_param_ = json["cluster_param"];
		}

		std::uint64_t get_id() const
		{
			return this->id_;
		}

		std::uint64_t get_player_id() const
		{
			return this->player_id_;
		}

		std::uint32_t get_area_id() const
		{
			return this->area_id_;
		}

		std::uint32_t get_platform_count() const
		{
			return this->platform_count_;
		}

		std::uint32_t get_security_rank() const
		{
			return this->security_rank_;
		}

		std::uint32_t get_construct_param() const
		{
			return this->construct_param_;
		}

		std::chrono::microseconds get_creation_time() const
		{
			return this->create_date_;
		}

		nlohmann::json& get_cluster_param()
		{
			return this->cluster_param_;
		}

		std::uint64_t get_index() const
		{
			return this->index_;
		}

	private:
		std::uint64_t id_;
		std::uint64_t player_id_;
		std::uint64_t index_;
		std::uint32_t area_id_;
		std::uint32_t platform_count_;
		std::uint32_t security_rank_;
		std::uint32_t construct_param_;
		std::chrono::microseconds create_date_;
		nlohmann::json cluster_param_;

	};

	std::vector<fob> get_fob_list(const std::uint64_t player_id);
	void create(const std::uint64_t player_id, const std::uint32_t area_id);
	void sync_data(const std::uint64_t player_id, std::vector<fob>& fobs);
	std::optional<fob> get_fob(const std::uint64_t id);
}
