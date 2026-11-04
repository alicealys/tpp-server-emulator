#pragma once

#include "../database.hpp"

namespace database::challenge_tasks
{
	constexpr const auto concurrent_task_count = 8;

	struct challenge_task_t
	{
		struct reward_t
		{
			std::uint8_t bottom_type;
			std::uint32_t rate;
			std::uint8_t section;
			std::uint8_t type;
			std::uint32_t value;
		};

		std::uint32_t mission_id;
		std::uint16_t task_type_id;
		std::uint8_t threshold;
		std::uint32_t id;
		reward_t reward;

		void to_json(nlohmann::json& data) const;
	};

	std::vector<challenge_task_t> get_current_challenge_tasks(std::chrono::seconds& start, std::chrono::seconds& end);
	void get_date_range(std::chrono::seconds& start, std::chrono::seconds& end);
	std::uint32_t get_current_version();

	class challenge_task_order
	{
	public:
		DEFINE_FIELD(order_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(task_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(status, sqlpp::integer_unsigned);
		DEFINE_FIELD(expire_date, sqlpp::time_point);
		DEFINE_TABLE(challenge_task_orders, order_id_field_t, player_id_field_t, task_id_field_t, status_field_t, expire_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		challenge_task_order(const sqlpp::result_row_t<Args...>& row)
		{
			this->order_id_ = row.order_id;
			this->player_id_ = row.player_id;
			this->task_id_ = static_cast<std::uint32_t>(row.task_id);
			this->status_ = static_cast<std::uint32_t>(row.status);
			this->expire_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.expire_date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, order_id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, task_id);
		GET_FIELD_H(std::uint32_t, status);
		GET_FIELD_H(std::chrono::seconds, expire_date);

	};

	bool create_order(const std::uint64_t player_id, const std::uint32_t task_id, const std::uint32_t status, const std::chrono::seconds expire_date);
	bool set_order_status(const std::uint64_t order_id, const std::uint32_t status);
	std::vector<challenge_task_order> get_order_list(const std::uint64_t player_id);

	void delete_player_data(const std::uint64_t player_id);
}
