#pragma once

#include "../database.hpp"

namespace database::combat_deployments
{
	struct mission_reward_t
	{
		std::uint8_t bottom_type;
		std::uint8_t mecha_type;
		std::uint8_t type;
		std::uint8_t section;
		std::uint32_t value;
		std::uint32_t rate;
	};

	struct mission_t
	{
		std::uint8_t category;
		std::uint8_t combat_count;
		std::uint8_t combat_rank;
		std::uint8_t dead_rate;
		std::uint8_t is_campaign;
		std::uint8_t max_dead_rate;
		std::uint8_t min_dead_rate;
		std::uint8_t max_win_rate;
		std::uint8_t min_win_rate;
		std::uint8_t name_key;
		std::uint8_t reward;
		std::uint8_t section;
		std::uint8_t section_count;
		std::uint8_t section_rank;
		std::uint32_t id;
		std::uint32_t time;
		std::int32_t latitude;
		std::int32_t longitude;
		std::vector<mission_reward_t> rewards;
	};

	const std::vector<mission_t>& get_mission_list();
	std::optional<mission_t> get_mission(const std::uint32_t mission_id);

	struct deployment_info_t
	{
		std::uint8_t armored;
		std::uint8_t battle_gear;
		std::uint8_t car;
		std::uint8_t combat_count;
		std::uint8_t combat_rank_bottom;
		std::uint8_t combat_rank_top;
		std::uint8_t dead_rate;
		std::uint8_t staff_power;
		std::uint8_t sub_count;
		std::uint8_t sub_rank_bottom;
		std::uint8_t sub_rank_top;
		std::uint8_t tank;
		std::uint8_t team_id;
		std::uint8_t truck;
		std::uint8_t walker_gear;
		std::uint8_t win_rate;
		std::uint32_t seed;
		std::uint32_t team_power;
	};

	class combat_deployment
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(mission_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(deployment_info, sqlpp::binary);
		DEFINE_FIELD(is_win, sqlpp::boolean);
		DEFINE_FIELD(start_date, sqlpp::time_point);
		DEFINE_FIELD(end_date, sqlpp::time_point);
		DEFINE_TABLE(combat_deployments, id_field_t, player_id_field_t, mission_id_field_t,
			deployment_info_field_t, is_win_field_t, start_date_field_t, end_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		combat_deployment(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->mission_id_ = static_cast<std::uint32_t>(row.mission_id);
			this->is_win_ = static_cast<bool>(row.is_win);
			this->start_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.start_date.value().time_since_epoch());
			this->end_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.end_date.value().time_since_epoch());

			const auto deployment_info_str = row.deployment_info.value();
			if (deployment_info_str.size() == sizeof(deployment_info_t))
			{
				std::memcpy(&this->deployment_info_, deployment_info_str.data(), sizeof(deployment_info_t));
			}
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, mission_id);
		GET_FIELD_H(bool, is_win);
		GET_FIELD_H(std::chrono::seconds, start_date);
		GET_FIELD_H(std::chrono::seconds, end_date);

		deployment_info_t& get_deployment_info()
		{
			return this->deployment_info_;
		}

		const deployment_info_t& get_deployment_info() const
		{
			return this->deployment_info_;
		}

		bool completed() const;
		std::chrono::seconds get_time_left() const;

	private:
		deployment_info_t deployment_info_{};

	};

	std::vector<combat_deployment> get_deployments(const std::uint64_t player_id);
	std::optional<combat_deployment> get_deployment(const std::uint64_t player_id, const std::uint32_t mission_id);
	bool deploy_mission(const std::uint64_t player_id, const std::uint32_t mission_id, const std::uint32_t time, 
		const deployment_info_t& deployment_info, const bool is_win);
	bool delete_deployment(const std::uint64_t player_id, const std::uint32_t mission_id);
	bool delete_all_deployments(const std::uint64_t player_id);
	bool complete_deployment(const std::uint64_t player_id, const std::uint32_t mission_id);
	void delete_player_data(const std::uint64_t player_id);
}
