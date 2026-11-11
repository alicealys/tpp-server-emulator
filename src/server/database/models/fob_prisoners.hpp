#pragma once

#include "../database.hpp"
#include "../utils.hpp"

#include "player_data.hpp"

namespace database::fob_prisoners
{
	constexpr const std::chrono::seconds prisoner_hold_time = 24h * 3;
	constexpr const auto persuade_param_max = 15u;
	
	using prisoner_param_t = database_struct<game::staff_fields_t>;

	enum prisoner_state_t : std::uint32_t
	{
		state_none = 0,
		state_captured = 1,
		state_rescued = 2,
	};

	struct rescue_t
	{
		std::uint64_t player_id;
		std::size_t value;
	};

	class fob_prisoner
	{
	public:
		friend class item_list;

		DEFINE_FIELD(prisoner_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(owner_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(rescuer_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(prisoner_state, sqlpp::integer_unsigned);
		DEFINE_FIELD(staff_param, sqlpp::text);
		DEFINE_FIELD(capture_date, sqlpp::time_point);
		DEFINE_TABLE(fob_prisoners, 
			prisoner_id_field_t,
			attacker_id_field_t,
			owner_id_field_t,
			rescuer_id_field_t,
			prisoner_state_field_t,
			staff_param_field_t,
			capture_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		fob_prisoner(const sqlpp::result_row_t<Args...>& row)
		{
			this->prisoner_id_ = row.prisoner_id;
			this->attacker_id_ = row.attacker_id;
			this->owner_id_ = row.owner_id;
			this->rescuer_id_ = row.rescuer_id;
			this->state_ = static_cast<std::uint32_t>(row.prisoner_state);
			this->staff_param_.deserialize(row.staff_param.value());
			this->capture_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.capture_date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, prisoner_id);
		GET_FIELD_H(std::uint64_t, attacker_id);
		GET_FIELD_H(std::uint64_t, owner_id);
		GET_FIELD_H(std::uint64_t, rescuer_id);
		GET_FIELD_H(std::uint32_t, state);
		GET_FIELD_H(prisoner_param_t, staff_param);
		GET_FIELD_H(std::chrono::seconds, capture_date);

		std::uint32_t get_persuade_param() const;
		void to_json(nlohmann::json& data) const;

	};

	void add_prisoners(const std::uint64_t attacker_id, const std::uint64_t owner_id, const std::vector<game::staff_t>& prisoners);

	std::vector<fob_prisoner> get_captive_list(const std::uint64_t player_id);
	std::vector<fob_prisoner> get_free_list(const std::uint64_t player_id);
	std::vector<fob_prisoner> get_rescued_list(const std::uint64_t owner_id);
	std::vector<rescue_t> get_rescued_counts(const std::uint64_t owner_id);
	std::vector<fob_prisoner> get_rescueable_list(const std::uint64_t owner_id, const std::uint64_t attacker_id);

	void set_prisoner_states(const std::vector<std::uint64_t>& list, const std::uint32_t state);
	void remove_prisoners(const std::vector<std::uint64_t>& list);
	void clear_rescues(const std::uint64_t owner_id);

	void direct_rescue(const std::uint64_t rescuer_id);
	void indirect_rescue(const std::uint64_t rescuer_id);

	void delete_player_data(const std::uint64_t player_id);
}
