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

	struct team_info_t
	{
		std::uint32_t current_4wd_count;
		std::uint32_t current_armored_count;
		std::uint32_t current_tank_count;
		std::uint32_t current_truck_count;
		std::uint32_t current_walkergear_count;
		std::uint32_t need_4wd_count;
		std::uint32_t need_armored_count;
		std::uint32_t need_tank_count;
		std::uint32_t need_truck_count;
		std::uint32_t need_walkergear_count;
		std::uint32_t current_staff_counts[6];
		std::uint32_t need_staff_counts[6];
		std::uint8_t staff_ranks[6];

		bool parse(nlohmann::json& data);
		std::uint32_t calc_replace_cost() const;
	};

	struct deploy_exec_info_t
	{
		std::uint8_t win_rate;
		team_info_t team_info;

		bool parse(nlohmann::json& data);
	};

	struct deploy_result_t
	{
		std::uint8_t is_win;
		std::uint32_t armored_east;
		std::uint32_t armored_east_rocket;
		std::uint32_t armored_west;
		std::uint32_t armored_west_wheeled;
		std::uint32_t car_east;
		std::uint32_t car_west;
		std::uint32_t tank_east;
		std::uint32_t tank_west;
		std::uint32_t truck_east;
		std::uint32_t truck_west;
		std::uint32_t walker_gear_cfa;
		std::uint32_t walker_gear_cfa_suppot;
		std::uint32_t walker_gear_proto;
		std::uint32_t walker_gear_soviet;
		std::uint32_t walker_gear_soviet_support;
		std::vector<game::reward_t> rewards;

		void to_json(nlohmann::json& data) const;
	};

	struct deploy_settings_t
	{
		struct reward_section_t
		{
			std::uint8_t threshold;
			std::vector<game::reward_t> rewards;
		};

		struct attack_t
		{
			struct grade_param_t
			{
				struct rate_t
				{
					struct values_t
					{
						std::uint8_t win_rate;
						std::uint8_t dead_rate;
						std::uint8_t section;
						std::uint32_t section_count;
						std::uint8_t section_rank;
					};

					std::uint8_t threshold;
					values_t values;
				};

				std::uint32_t armored_count;
				std::uint32_t car_count;
				std::uint32_t tank_count;
				std::uint32_t truck_count;
				std::uint32_t walker_gear_count;
				std::uint32_t info_key_id;
				std::uint32_t name_key_id;
				std::array<std::uint8_t, 16> damage_value;
				std::vector<rate_t> rates;

				const rate_t& get_rate(const std::uint8_t security_level) const;
			};

			std::uint8_t attack_type;
			std::vector<grade_param_t> grade_params;
		};

		struct replace_costs_t
		{
			float armored;
			float car;
			float tank;
			float truck;
			float walker_gear;
			std::array<float, 6> staff_counts;
			std::array<std::array<float, 10>, 6> staff_ranks;
		};

		std::uint8_t base_dead_rate;
		std::uint8_t base_win_rate;
		std::uint8_t max_dead_rate;
		std::uint8_t max_win_rate;
		std::uint8_t min_dead_rate;
		std::uint8_t min_grade;
		std::uint8_t min_guard_soldier_count;
		std::uint8_t min_security_device_count;
		std::uint8_t min_win_rate;
		replace_costs_t replace_costs;
		std::vector<reward_section_t> reward_sections;
		std::vector<attack_t> attacks;

		const reward_section_t& get_reward_section(const std::uint8_t security_level) const;
	};

	struct deploy_info_t
	{
		struct attack_param_t
		{
			struct grade_param_t
			{
				std::uint32_t armored_count;
				std::uint32_t car_count;
				std::uint32_t tank_count;
				std::uint32_t truck_count;
				std::uint32_t walker_gear_count;
				std::uint32_t info_key_id;
				std::uint32_t name_key_id;
				std::array<std::uint8_t, 16> damage_target;
				std::array<std::uint8_t, 16> damage_value;
				std::uint8_t win_rate;
				std::uint8_t dead_rate;
				std::uint8_t section;
				std::uint32_t section_count;
				std::uint8_t section_rank;
				std::uint8_t is_valid;
			};

			std::uint8_t attack_type;
			std::vector<grade_param_t> grade_params;
		};

		std::uint8_t base_dead_rate;
		std::uint8_t base_win_rate;
		std::uint8_t free_position_camera_count;
		std::uint8_t free_position_mine_count;
		std::uint8_t max_dead_rate;
		std::uint8_t max_win_rate;
		std::uint8_t min_dead_rate;
		std::uint8_t min_grade;
		std::uint8_t min_guard_soldier_count;
		std::uint8_t min_security_device_count;
		std::uint8_t min_win_rate;
		std::vector<game::reward_t> primary_reward;
		std::vector<attack_param_t> attack_params;

		void generate(const game::fob_cluster_param_single_t& cluster_param, const std::uint8_t security_level);
		void to_json(nlohmann::json& data) const;
	};

	bool perform_deploy(const game::fob_cluster_param_single_t& cluster_param, const deploy_exec_info_t& exec_info, deploy_result_t& result);

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

		std::uint64_t get_fob_id() const;
		std::uint8_t get_cluster_index() const;
		std::chrono::seconds get_expire_date() const;

	private:
		deploy_params_t params_{};

	};

	bool deploy_assist(const std::uint64_t player_id, const deploy_params_t& params);
	bool delete_deploy_to_fob(const std::uint64_t player_id, const std::uint64_t fob_id);
	bool delete_deployments(const std::uint64_t player_id);
	std::optional<fob_deploy_assist> get_current_deploy(const std::uint64_t player_id);

	void delete_player_data(const std::uint64_t player_id);
}
