#pragma once

#include "../database.hpp"

#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::fobs
{
	DEFINE_FIELD(id, sqlpp::integer_unsigned);
	DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(area_id, sqlpp::integer_unsigned);
	DEFINE_FIELD(cluster_param, sqlpp::text);
	DEFINE_FIELD(construct_param, sqlpp::integer_unsigned);
	DEFINE_FIELD(create_date, sqlpp::time_point);
	DEFINE_TABLE(fobs, id_field_t, player_id_field_t, area_id_field_t, 
		cluster_param_field_t, construct_param_field_t, create_date_field_t);

	nlohmann::json& get_area_list();
	std::optional<nlohmann::json> get_area(const std::uint32_t area_id);

	class fob
	{
	public:
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
			this->area_id_ = row.area_id;
			this->construct_param_ = row.construct_param;
			this->create_date_ = row.create_date.value().time_since_epoch();
		}

		std::uint32_t get_id() const
		{
			return this->id_;
		}

		std::uint32_t get_area_id() const
		{
			return this->area_id_;
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

	private:
		std::uint32_t id_;
		std::uint32_t player_id_;
		std::uint32_t area_id_;
		std::uint32_t construct_param_;
		std::chrono::microseconds create_date_;
		nlohmann::json cluster_param_;

	};

	std::vector<fob> get_fob_list(const std::uint64_t player_id);
	void create(const std::uint64_t player_id, const std::uint32_t area_id);
}
