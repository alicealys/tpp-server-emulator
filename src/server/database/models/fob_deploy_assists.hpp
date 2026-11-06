#pragma once

#include "../database.hpp"
#include "../utils.hpp"
#include "game/game.hpp"

namespace database::fob_deploy_assists
{
	constexpr const auto deploy_duration = 24h;

	struct damage_values_internal_t
	{
		std::uint8_t value[16];
	};

	using damage_values_t = database_struct<damage_values_internal_t>;

	struct deploy_params_t
	{
		std::uint8_t cluster_index;
		std::uint64_t fob_id;
		std::chrono::seconds expire_date;
		damage_values_t damage_values;

		bool parse(nlohmann::json& data);
		void set_expire_in(const std::chrono::seconds);

		void to_json(nlohmann::json& data) const;
		void apply(game::fob_cluster_param_t& cluster) const;
	};

	struct deploy_exec_info_t
	{
		struct team_info_t
		{
			std::uint8_t current_4wd_count;
			std::uint8_t current_armored_count;
			std::uint8_t current_tank_count;
			std::uint8_t current_truck_count;
			std::uint8_t current_walkergear_count;
			std::uint8_t need_4wd_count;
			std::uint8_t need_armored_count;
			std::uint8_t need_tank_count;
			std::uint8_t need_truck_count;
			std::uint8_t need_walkergear_count;
			std::uint8_t current_staff_counts[6];
			std::uint8_t need_staff_counts[6];
			std::uint8_t staff_ranks[6];
		};

		std::uint8_t win_rate;
		team_info_t team_info;

		bool parse(nlohmann::json& data);
	};

	struct deploy_result_t
	{
		std::uint8_t is_win;
		std::uint8_t armored_east;
		std::uint8_t armored_east_rocket;
		std::uint8_t armored_west;
		std::uint8_t armored_west_wheeled;
		std::uint8_t car_east;
		std::uint8_t car_west;
		std::uint8_t tank_east;
		std::uint8_t tank_west;
		std::uint8_t truck_east;
		std::uint8_t truck_west;
		std::uint8_t walker_gear_cfa;
		std::uint8_t walker_gear_cfa_suppot;
		std::uint8_t walker_gear_proto;
		std::uint8_t walker_gear_soviet;
		std::uint8_t walker_gear_soviet_support;
		std::vector<game::reward_t> rewards;

		void to_json(nlohmann::json& data) const;
	};

	bool perform_deploy(const deploy_exec_info_t& exec_info, deploy_result_t& result);

	class fob_deploy_assist
	{
	public:
		DEFINE_FIELD(deploy_assist_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(fob_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(cluster_index, sqlpp::integer_unsigned);
		DEFINE_FIELD(damage_values, sqlpp::text);
		DEFINE_FIELD(expire_date, sqlpp::time_point);
		DEFINE_TABLE(fob_deploy_assists, 
			deploy_assist_id_field_t, 
			player_id_field_t, 
			fob_id_field_t,
			cluster_index_field_t,
			damage_values_field_t,
			expire_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		fob_deploy_assist(const sqlpp::result_row_t<Args...>& row)
		{
			this->deploy_assist_id_ = row.deploy_assist_id;
			this->player_id_ = row.player_id;
			this->params_.fob_id = row.fob_id;
			this->params_.cluster_index = static_cast<std::uint8_t>(row.cluster_index);
			this->params_.expire_date = std::chrono::duration_cast<std::chrono::seconds>(row.expire_date.value().time_since_epoch());
			this->params_.damage_values.deserialize(row.damage_values.value());
		}

		void to_json(nlohmann::json& data) const;
		void apply(game::fob_cluster_param_t& cluster) const;

		GET_FIELD_H(std::uint64_t, deploy_assist_id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint64_t, fob_id);
		GET_FIELD_H(std::uint8_t, cluster_index);
		GET_FIELD_H(std::chrono::seconds, expire_date);

	private:
		deploy_params_t params_{};

	};

	bool deploy_assist(const std::uint64_t player_id, const deploy_params_t& params);
	bool delete_deploy_to_fob(const std::uint64_t player_id, const std::uint64_t fob_id);
	bool delete_deployments(const std::uint64_t player_id);
	std::optional<fob_deploy_assist> get_current_deploy(const std::uint64_t player_id);

	void delete_player_data(const std::uint64_t player_id);
}
