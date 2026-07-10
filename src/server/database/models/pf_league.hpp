#pragma once

#include "../database.hpp"
#include "player_data.hpp"
#include "fobs.hpp"
#include "fob_events.hpp"

namespace database::pf_league
{
	enum pf_param_type_t : std::uint32_t
	{
		offensive_capability_sum = 0,
		offensive_capability_combat = 1,
		offensive_capability_grade = 2,
		offensive_capability_nuclear = 3,
		offensive_capability_total_units = 4,
		offensive_capability_vehicles = 5,
		offensive_capability_fight_vehicles = 6,
		offensive_capability_walker_gears = 7,
		offensive_capability_plants = 8,
		offensive_capability_parasites = 9,
		offensive_capability_skill1 = 10,
		offensive_capability_skill2 = 11,
		offensive_capability_skill3 = 12,
		offensive_capability_bonus = 13,

		offensive_durability_sum = 0,
		offensive_durability_rank_e = 1,
		offensive_durability_rank_d = 2,
		offensive_durability_rank_c = 3,
		offensive_durability_rank_b = 4,
		offensive_durability_rank_a = 5,
		offensive_durability_rank_ap = 6,
		offensive_durability_rank_app = 7,
		offensive_durability_rank_s = 8,
		offensive_durability_rank_sp = 9,
		offensive_durability_rank_spp = 10,
		offensive_durability_skill1 = 11,
		offensive_durability_skill2 = 12,
		offensive_durability_skill3 = 13,
		offensive_durability_unprocessed_materials = 14,
		offensive_durability_processed_materials = 15,
		offensive_durability_gmp = 16,

		defensive_capability_sum = 0,
		defensive_capability_total_defense = 1,
		defensive_capability_security = 2,
		defensive_capability_platforms = 3,
		defensive_capability_nuclear = 4,
		defensive_capability_total_units = 5,
		defensive_capability_support = 6,
		defensive_capability_emplacements = 7,
		defensive_capability_mortars = 8,
		defensive_capability_machine_guns = 9,
		defensive_capability_walker_gears = 10,
		defensive_capability_plants = 11,
		defensive_capability_parasites = 12,
		defensive_capability_skill1 = 13,
		defensive_capability_skill2 = 14,
		defensive_capability_skill3 = 15,
		defensive_capability_bonus = 16,

		defensive_durability_sum = 0,
		defensive_durability_platforms = 1,
		defensive_durability_nuclear = 2,
		defensive_durability_rank_e = 3,
		defensive_durability_rank_d = 4,
		defensive_durability_rank_c = 5,
		defensive_durability_rank_b = 6,
		defensive_durability_rank_a = 7,
		defensive_durability_rank_ap = 8,
		defensive_durability_rank_app = 9,
		defensive_durability_rank_s = 10,
		defensive_durability_rank_sp = 11,
		defensive_durability_rank_spp = 12,
		defensive_durability_skill1 = 13,
		defensive_durability_skill2 = 14,
		defensive_durability_skill3 = 15,
		defensive_durability_unprocessed_materials = 16,
		defensive_durability_processed_materials = 17,
		defensive_durability_gmp = 18,
	};

	struct player_pf_params_t
	{
		struct category_t
		{
			std::uint32_t elements[23];
		};

		category_t offensive_capability;
		category_t offensive_durability;
		category_t defensive_capability;
		category_t defensive_durability;
		std::uint32_t security_level;
	};

	struct player_pf_data_t
	{
		game::motherbase_t motherbase;
		std::uint32_t server_gmp;
		std::uint32_t local_gmp;
		std::uint32_t cumulative_grade;
		database::player_data::unit_levels_t unit_levels;
		database::player_data::unit_counts_t unit_counts;
		database::player_data::resource_arrays_t resources;
		database::player_data::staff_array_container staff;
		std::vector<database::fobs::fob> fobs;
	};

	struct pf_point_info_t
	{
		float baseline;
		float group;
	};

	void calculate_pf_params(player_pf_data_t& in_data, player_pf_params_t& out_params);
	bool calculate_pf_params(const std::uint64_t player_id, player_pf_data_t& pf_data, player_pf_params_t& out_params);
	const std::vector<pf_point_info_t>& get_pf_points_table();

	const std::vector<fob_events::point_exchange_param_t>& get_point_exchange_params();

	enum pf_league_state_t
	{
		league_state_none = 0,
		league_state_initial = 1,
		league_state_running = 2,
		league_state_completed = 3,
		league_state_destroy = 4,
		league_state_dead = 10
	};
	
	constexpr const auto pf_bracket_size = 16u;

	class pf_league
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(state, sqlpp::integer_unsigned);
		DEFINE_FIELD(start_date, sqlpp::time_point);
		DEFINE_FIELD(end_date, sqlpp::time_point);
		DEFINE_TABLE(pf_leagues, id_field_t, state_field_t, start_date_field_t, end_date_field_t);

		inline static table_t table;

		template <typename ...Args>
		pf_league(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->state_ = static_cast<std::uint32_t>(row.state);
			this->start_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.start_date.value().time_since_epoch());
			this->end_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.end_date.value().time_since_epoch());
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint32_t, state);
		GET_FIELD_H(std::chrono::seconds, start_date);
		GET_FIELD_H(std::chrono::seconds, end_date);

	};
	
	enum pf_bracket_state_t
	{
		bracket_state_none = 0,
		bracket_state_done = 1,
	};

	class pf_bracket
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(state, sqlpp::integer_unsigned);
		DEFINE_TABLE(pf_brackets, id_field_t, league_id_field_t, state_field_t);

		inline static table_t table;

		template <typename ...Args>
		pf_bracket(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->league_id_ = row.league_id;
			this->state_ = row.state;
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, league_id);
		GET_FIELD_H(std::uint32_t, state);

	};

	struct battle_result_stats_t
	{
		std::int32_t victory_points;
		std::uint32_t win;
		std::uint32_t narrow_win;
		std::uint32_t attack_win;
		std::uint32_t defense_win;
		std::uint32_t lose;
		std::uint32_t narrow_lose;
		std::uint32_t attack_lose;
		std::uint32_t defense_lose;
	};

	class pf_competitor
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(player_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(bracket_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(bracket_rank, sqlpp::integer_unsigned);
		DEFINE_FIELD(bracket_rank_prev, sqlpp::integer_unsigned);
		DEFINE_FIELD(victory_points, sqlpp::integer);
		DEFINE_FIELD(win, sqlpp::integer_unsigned);
		DEFINE_FIELD(narrow_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(attack_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(defense_win, sqlpp::integer_unsigned);
		DEFINE_FIELD(lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(narrow_lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(attack_lose, sqlpp::integer_unsigned);
		DEFINE_FIELD(defense_lose, sqlpp::integer_unsigned);
		DEFINE_TABLE(pf_competitors, id_field_t, player_id_field_t, league_id_field_t,
			bracket_id_field_t, bracket_rank_field_t, bracket_rank_prev_field_t, victory_points_field_t,
			win_field_t,
			narrow_win_field_t,
			attack_win_field_t,
			defense_win_field_t,
			lose_field_t,
			narrow_lose_field_t,
			attack_lose_field_t,
			defense_lose_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		pf_competitor(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->player_id_ = row.player_id;
			this->league_id_ = row.league_id;
			this->bracket_id_ = row.bracket_id;
			this->bracket_rank_ = static_cast<std::uint32_t>(row.bracket_rank);
			this->bracket_rank_prev_ = static_cast<std::uint32_t>(row.bracket_rank_prev);

			this->victory_points_ = static_cast<std::int32_t>(row.victory_points);

			this->win_ = static_cast<std::uint32_t>(row.win);
			this->narrow_win_ = static_cast<std::uint32_t>(row.narrow_win);
			this->attack_win_ = static_cast<std::uint32_t>(row.attack_win);
			this->defense_win_ = static_cast<std::uint32_t>(row.defense_win);

			this->lose_ = static_cast<std::uint32_t>(row.lose);
			this->narrow_lose_ = static_cast<std::uint32_t>(row.narrow_lose);
			this->attack_lose_ = static_cast<std::uint32_t>(row.attack_lose);
			this->defense_lose_ = static_cast<std::uint32_t>(row.defense_lose);
		}

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, player_id);
		GET_FIELD_H(std::uint64_t, league_id);
		GET_FIELD_H(std::uint64_t, bracket_id);
		GET_FIELD_H(std::uint32_t, bracket_rank);
		GET_FIELD_H(std::uint32_t, bracket_rank_prev);
		GET_FIELD_H(std::int32_t, victory_points);
		GET_FIELD_H(std::uint32_t, win);
		GET_FIELD_H(std::uint32_t, narrow_win);
		GET_FIELD_H(std::uint32_t, attack_win);
		GET_FIELD_H(std::uint32_t, defense_win);
		GET_FIELD_H(std::uint32_t, lose);
		GET_FIELD_H(std::uint32_t, narrow_lose);
		GET_FIELD_H(std::uint32_t, attack_lose);
		GET_FIELD_H(std::uint32_t, defense_lose);
	};

	enum battle_winner_state_t
	{
		battle_winner_none = 0,
		battle_winner_attacker = 1,
		battle_winner_defender = 2,
		battle_winner_draw = 3,
	};

	struct battle_result_t
	{
		battle_winner_state_t state;
		std::int32_t attacker_points;
		std::int32_t defender_points;
	};

	class pf_battle
	{
	public:
		DEFINE_FIELD(id, sqlpp::integer_unsigned);
		DEFINE_FIELD(league_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(bracket_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(section, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_points, sqlpp::integer);
		DEFINE_FIELD(defender_points, sqlpp::integer);
		DEFINE_FIELD(attacker_buff, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_buff, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_level, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_level, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_capability, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_capability, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_durability, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_durability, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_grade, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_security, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_staff, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_staff, sqlpp::integer_unsigned);
		DEFINE_FIELD(attacker_nuclear, sqlpp::integer_unsigned);
		DEFINE_FIELD(defender_nuclear, sqlpp::integer_unsigned);
		DEFINE_FIELD(winner_state, sqlpp::integer_unsigned);
		DEFINE_FIELD(date, sqlpp::time_point);
		DEFINE_TABLE(pf_battles, 
			id_field_t, 
			league_id_field_t, 
			bracket_id_field_t,
			section_field_t,
			attacker_id_field_t, defender_id_field_t,
			attacker_points_field_t, defender_points_field_t,
			attacker_buff_field_t, defender_buff_field_t,
			attacker_level_field_t, defender_level_field_t,
			attacker_capability_field_t, defender_capability_field_t,
			attacker_durability_field_t, defender_durability_field_t,
			attacker_grade_field_t, defender_security_field_t,
			attacker_staff_field_t, defender_staff_field_t,
			attacker_nuclear_field_t, defender_nuclear_field_t,
			winner_state_field_t, 
			date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		pf_battle(const sqlpp::result_row_t<Args...>& row)
		{
			this->id_ = row.id;
			this->league_id_ = row.league_id;
			this->bracket_id_ = row.bracket_id;
			this->section_ = static_cast<std::uint32_t>(row.section);
			this->attacker_id_ = row.attacker_id;
			this->defender_id_ = row.defender_id;
			this->attacker_points_ = static_cast<std::int32_t>(row.attacker_points);
			this->defender_points_ = static_cast<std::int32_t>(row.defender_points);
			this->attacker_buff_ = static_cast<std::uint32_t>(row.attacker_buff);
			this->defender_buff_ = static_cast<std::uint32_t>(row.defender_buff);
			this->attacker_level_ = static_cast<std::uint32_t>(row.attacker_level);
			this->defender_level_ = static_cast<std::uint32_t>(row.defender_level);
			this->attacker_capability_ = static_cast<std::uint32_t>(row.attacker_capability);
			this->defender_capability_ = static_cast<std::uint32_t>(row.defender_capability);
			this->attacker_durability_ = static_cast<std::uint32_t>(row.attacker_durability);
			this->defender_durability_ = static_cast<std::uint32_t>(row.defender_durability);
			this->attacker_grade_ = static_cast<std::uint32_t>(row.attacker_grade);
			this->defender_security_ = static_cast<std::uint32_t>(row.defender_security);
			this->attacker_staff_ = static_cast<std::uint32_t>(row.attacker_staff);
			this->defender_staff_ = static_cast<std::uint32_t>(row.defender_staff);
			this->attacker_nuclear_ = static_cast<std::uint32_t>(row.attacker_nuclear);
			this->defender_nuclear_ = static_cast<std::uint32_t>(row.defender_nuclear);
			this->winner_state_ = static_cast<std::uint32_t>(row.winner_state);
			this->date_ = std::chrono::duration_cast<std::chrono::seconds>(row.date.value().time_since_epoch());
		}

		pf_battle() = default;

		GET_FIELD_H(std::uint64_t, id);
		GET_FIELD_H(std::uint64_t, league_id);
		GET_FIELD_H(std::uint64_t, bracket_id);
		GET_FIELD_H(std::uint32_t, section);
		GET_FIELD_H(std::uint64_t, attacker_id);
		GET_FIELD_H(std::uint64_t, defender_id);
		GET_FIELD_H(std::int32_t, attacker_points);
		GET_FIELD_H(std::int32_t, defender_points);
		GET_FIELD_H(std::uint32_t, attacker_buff);
		GET_FIELD_H(std::uint32_t, defender_buff);
		GET_FIELD_H(std::uint32_t, attacker_capability);
		GET_FIELD_H(std::uint32_t, defender_capability);
		GET_FIELD_H(std::uint32_t, attacker_level);
		GET_FIELD_H(std::uint32_t, defender_level);
		GET_FIELD_H(std::uint32_t, attacker_durability);
		GET_FIELD_H(std::uint32_t, defender_durability);
		GET_FIELD_H(std::uint32_t, attacker_grade);
		GET_FIELD_H(std::uint32_t, defender_security);
		GET_FIELD_H(std::uint32_t, attacker_staff);
		GET_FIELD_H(std::uint32_t, defender_staff);
		GET_FIELD_H(std::uint32_t, attacker_nuclear);
		GET_FIELD_H(std::uint32_t, defender_nuclear);
		GET_FIELD_H(std::uint32_t, winner_state);
		GET_FIELD_H(std::chrono::seconds, date);

		void update_params(player_pf_data_t& attacker_data, player_pf_data_t& defender_data);
		void update_params(
			const player_pf_data_t& attacker_data, const player_pf_data_t& defender_data,
			const player_pf_params_t& attacker_params, const player_pf_params_t& defender_params);

	};

	std::optional<pf_league> get_current_pf_league();
	std::optional<pf_competitor> get_player_competitor_instance(const std::uint64_t league_id, const std::uint64_t player_id);
	std::vector<pf_competitor> get_players_in_bracket(const std::uint64_t bracket_id);
	std::vector<pf_battle> get_player_battles(const std::uint64_t bracket_id, const std::uint64_t player_id);
	void inc_battle_buff(const std::uint64_t battle_id, const std::uint32_t attacker_buff, const std::uint32_t defender_buff, const bool inc);
}
