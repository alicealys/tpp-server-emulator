#pragma once

#include "../database.hpp"

#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::event_rankings
{
	enum event_class
	{
		class_fob,
		class_fob_event,
		class_league,
		class_league_event
	};

	enum event_type
	{
		fob_event_start = 0,
		ep_earned = 0,
		most_ep_in_mission = 1,
		cores_reached_defender = 2,
		cores_reached_no_defender = 3,
		damage_defender = 4,
		materials_stolen = 5,
		total_stealth = 6,
		unknown = 7,
		defense_deployments = 8,
		defense_deployments_support = 9,
		defense_deployments_support_received = 10,
		defender_neutralized_intruder = 11,
		successful_defenses = 12,
		times_neutralized_by_intruder = 13,
		fob_event_ranking = 14,
		fob_event_end = 14,

		league_event_start = 15,
		league_1 = 15,
		league_2 = 16,
		league_3 = 17,
		league_4 = 18,
		league_5 = 19,
		league_6 = 20,
		league_7 = 21,
		league_8 = 22,
		league_9 = 23,
		league_10 = 24,
		league_11 = 25,
		league_event_end = 25,

		event_count
	};

	enum event_ranking_lookup_type
	{
		lookup_best,  // best global
		lookup_around // around your placement
	};

	enum event_ranking_type
	{
		event_type_sneak,		// all time
		event_type_sneak_event // within period
	};

	std::optional<event_ranking_lookup_type> get_lookup_type_from_name(const std::string& name);
	std::optional<event_type> get_event_type_from_id(const std::uint32_t id, const std::string& type);

	class event_ranking
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(value, sqlpp::integer_unsigned);
		DEFINE_TABLE(event_rankings, id_field_t, player_id_field_t, event_id_field_t, player_rank_field_t, value_field_t);

		inline static table_t table;

		template <typename ...Args>
		event_ranking(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->event_id_ = static_cast<std::uint32_t>(row.event_id);
			this->rank_ = row.player_rank;
			this->value_ = static_cast<std::uint32_t>(row.value);
		}

		void set_league_grade(const std::uint32_t grade)
		{
			this->league_grade_ = grade;
		}

		void set_fob_grade(const std::uint32_t grade)
		{
			this->fob_grade_ = grade;
		}

		void set_account_id(const std::uint64_t account_id)
		{
			this->account_id_ = account_id;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint32_t, event_id);
		GET_FIELD_H(std::uint64_t, rank);
		GET_FIELD_H(std::uint32_t, value);
		GET_FIELD_H(std::uint32_t, fob_grade);
		GET_FIELD_H(std::uint32_t, league_grade);
		GET_FIELD_H(std::uint64_t, account_id);

	private:

	};

	void create_entries(const std::uint64_t player_id);

	bool set_event_value(const std::uint64_t player_id, const event_type event_id, const std::uint32_t value);
	bool increment_event_value(const std::uint64_t player_id, const event_type event_id, const std::uint32_t count);
	bool set_value_if_bigger(const std::uint64_t player_id, const event_type event_id, const std::uint32_t value);

	std::optional<std::uint64_t> get_player_rank(const std::uint64_t player_id, const event_type event_id);

	std::vector<event_ranking> get_entries(const event_type event_id, const std::uint64_t offset, const std::uint32_t num);

	bool reset_periodic_values();

	std::chrono::seconds get_last_update();
}
