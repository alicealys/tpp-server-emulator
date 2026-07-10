#include <std_include.hpp>

#include "pf_league.hpp"
#include "player_records.hpp"
#include "event_rankings.hpp"

#include "utils/encoding.hpp"

#include <utils/string.hpp>

namespace database::pf_league
{
	GET_FIELD_C(pf_league, std::uint64_t, id);
	GET_FIELD_C(pf_league, std::uint32_t, state);
	GET_FIELD_C(pf_league, std::chrono::seconds, start_date);
	GET_FIELD_C(pf_league, std::chrono::seconds, end_date);

	GET_FIELD_C(pf_bracket, std::uint64_t, id);
	GET_FIELD_C(pf_bracket, std::uint64_t, league_id);

	GET_FIELD_C(pf_competitor, std::uint64_t, id);
	GET_FIELD_C(pf_competitor, std::uint64_t, player_id);
	GET_FIELD_C(pf_competitor, std::uint64_t, league_id);
	GET_FIELD_C(pf_competitor, std::uint64_t, bracket_id);
	GET_FIELD_C(pf_competitor, std::uint32_t, bracket_rank);
	GET_FIELD_C(pf_competitor, std::uint32_t, bracket_rank_prev);
	GET_FIELD_C(pf_competitor, std::int32_t, victory_points);
	GET_FIELD_C(pf_competitor, std::uint32_t, win);
	GET_FIELD_C(pf_competitor, std::uint32_t, narrow_win);
	GET_FIELD_C(pf_competitor, std::uint32_t, attack_win);
	GET_FIELD_C(pf_competitor, std::uint32_t, defense_win);
	GET_FIELD_C(pf_competitor, std::uint32_t, lose);
	GET_FIELD_C(pf_competitor, std::uint32_t, narrow_lose);
	GET_FIELD_C(pf_competitor, std::uint32_t, attack_lose);
	GET_FIELD_C(pf_competitor, std::uint32_t, defense_lose);

	GET_FIELD_C(pf_battle, std::uint64_t, id);
	GET_FIELD_C(pf_battle, std::uint64_t, league_id);
	GET_FIELD_C(pf_battle, std::uint64_t, bracket_id);
	GET_FIELD_C(pf_battle, std::uint32_t, section);
	GET_FIELD_C(pf_battle, std::uint64_t, attacker_id);
	GET_FIELD_C(pf_battle, std::uint64_t, defender_id);
	GET_FIELD_C(pf_battle, std::int32_t, attacker_points);
	GET_FIELD_C(pf_battle, std::int32_t, defender_points);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_buff);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_buff);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_level);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_level);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_durability);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_durability);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_capability);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_capability);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_grade);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_security);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_staff);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_staff);
	GET_FIELD_C(pf_battle, std::uint32_t, attacker_nuclear);
	GET_FIELD_C(pf_battle, std::uint32_t, defender_nuclear);
	GET_FIELD_C(pf_battle, std::uint32_t, winner_state);
	GET_FIELD_C(pf_battle, std::chrono::seconds, date);

	void calculate_pf_params(player_pf_data_t& in_data, player_pf_params_t& out_params)
	{
		const auto exp_value = [](const float scalar, const std::uint32_t value)
		{
			auto value_f = static_cast<float>(value);
			return static_cast<std::uint32_t>(scalar * powf(value_f, 2.f) + scalar * value_f);
		};

		const auto linear_value = [](const float scalar, const std::uint32_t value)
		{
			auto value_f = static_cast<float>(value);
			return static_cast<std::uint32_t>(scalar * value_f);
		};

		const auto total_unit_levels = [&](const float scalar, const std::uint32_t exclude1, const std::uint32_t exclude2)
		{
			auto total = 0.f;
			for (auto i = 0u; i < game::unit_count; i++)
			{
				if (i == exclude1 || i == exclude2)
				{
					continue;
				}

				const auto value_f = static_cast<float>(in_data.unit_levels[i]);
				total += scalar * value_f;
			}
			return static_cast<std::uint32_t>(total);
		};

		const auto resource_capped_value = [&](const float scalar, const std::uint32_t cap, 
			const std::uint32_t resource_beg, const std::uint32_t resource_end)
		{
			auto total = 0u;

			for (auto i = resource_beg; i <= resource_end; i++)
			{
				total += in_data.resources[game::processed_server][i] +
					in_data.resources[game::processed_local][i];
			}

			const auto total_f = static_cast<float>(total);

			auto res = 0u;
			if (total <= 1000)
			{
				res = static_cast<std::uint32_t>(scalar * total_f);
			}
			else
			{
				const auto left = total_f - 1000.f;
				res = static_cast<std::uint32_t>(scalar * 1000.f + left * 1.f);
			}

			return std::min(cap, res);
		};

		const auto resource_value = [&](const float scalar, const bool processed, const std::uint32_t resource_beg, const std::uint32_t resource_end)
		{
			auto total = 0u;

			for (auto i = resource_beg; i <= resource_end; i++)
			{
				if (processed)
				{
					total += in_data.resources[game::processed_server][i] +
						in_data.resources[game::processed_local][i];
				}
				else
				{
					total += in_data.resources[game::unprocessed_server][i] +
						in_data.resources[game::unprocessed_local][i];
				}
			}

			return static_cast<std::uint32_t>(scalar * static_cast<float>(total));
		};

		std::uint32_t staff_rank_scores[10]{};
		staff_rank_scores[game::rank_e] = 1;
		staff_rank_scores[game::rank_d] = 30;
		staff_rank_scores[game::rank_c] = 60;
		staff_rank_scores[game::rank_b] = 90;
		staff_rank_scores[game::rank_a] = 120;
		staff_rank_scores[game::rank_ap] = 150;
		staff_rank_scores[game::rank_app] = 204;
		staff_rank_scores[game::rank_s] = 346;
		staff_rank_scores[game::rank_sp] = 543;
		staff_rank_scores[game::rank_spp] = 840;

		for (auto i = 0u; i < game::max_staff_count; i++)
		{
			const auto top_rank = game::calc_staff_top_rank(in_data.staff[i]);
			switch (in_data.staff[i].fields.status_sync.designation)
			{
			case game::des_combat:
				out_params.offensive_durability.elements[top_rank + 1] += staff_rank_scores[top_rank];
				break;
			case game::des_security:
				out_params.defensive_durability.elements[top_rank + 3] += staff_rank_scores[top_rank];
				break;
			}
		}

		auto total_platforms = 0u;
		auto unprocessed_materials = 0u;
		auto processed_materials = 0u;
		auto total_gmp = in_data.local_gmp + in_data.server_gmp;

		for (auto i = static_cast<std::uint32_t>(game::FUEL_RESOURCE); i <= game::PRECIOUS_METAL; i++)
		{
			unprocessed_materials += in_data.resources[game::unprocessed_local][i] + in_data.resources[game::unprocessed_server][i];
			processed_materials += in_data.resources[game::processed_local][i] + in_data.resources[game::processed_server][i];
		}

		for (const auto& fob : in_data.fobs)
		{
			const auto& param = fob.get_cluster_param();
			for (auto i = 0u; i < game::fob_sections_count; i++)
			{
				total_platforms += param.param[i].build.fields.platform_count;
				out_params.security_level += param.param[i].cluster_security.fields.level;
			}
		}

		for (auto i = 0u; i < game::fob_sections_count; i++)
		{
			total_platforms += in_data.motherbase.local_base_param[i].fields.platform_count;
		}

		/* offensive capability */

		out_params.offensive_capability.elements[offensive_capability_combat] = exp_value(3.f, in_data.unit_levels[game::unit_combat]);
		out_params.offensive_capability.elements[offensive_capability_grade] = linear_value(38.f, in_data.cumulative_grade);
		out_params.offensive_capability.elements[offensive_capability_nuclear] = linear_value(30000.f, in_data.resources[game::processed_server][game::NUCLEAR_WEAPON]);
		out_params.offensive_capability.elements[offensive_capability_total_units] = total_unit_levels(6.f, game::unit_combat, game::unit_combat);
		out_params.offensive_capability.elements[offensive_capability_vehicles] = resource_capped_value(5.f, 14000u, game::CAR_EAST, game::TRUCK_WEST);
		out_params.offensive_capability.elements[offensive_capability_fight_vehicles] = resource_capped_value(40.f, 49000u, game::ARMORED_VEHICLE_EAST, game::TANK_WEST);
		out_params.offensive_capability.elements[offensive_capability_walker_gears] = resource_capped_value(53.f, 49000u, game::WALKER_GEAR_PROTO_HEUY, game::WALKER_GEAR_CFA_SUPPORT);
		out_params.offensive_capability.elements[offensive_capability_plants] = resource_value(0.13f, true, game::WORM_WOOD, game::HAOMA);
		out_params.offensive_capability.elements[offensive_capability_parasites] = resource_value(0.33f, true, game::PARASITE_FOG, game::PARASITE_CURING);
		out_params.offensive_capability.elements[offensive_capability_skill1] = linear_value(100.f, in_data.motherbase.pf_skill_staff.ranger1_num);
		out_params.offensive_capability.elements[offensive_capability_skill2] = linear_value(200.f, in_data.motherbase.pf_skill_staff.ranger2_num);
		out_params.offensive_capability.elements[offensive_capability_skill3] = linear_value(300.f, in_data.motherbase.pf_skill_staff.ranger3_num);
		out_params.offensive_capability.elements[offensive_capability_bonus] = 0;

		for (auto i = static_cast<std::uint32_t>(offensive_capability_combat); i <= offensive_capability_bonus; i++)
		{
			out_params.offensive_capability.elements[offensive_capability_sum] += out_params.offensive_capability.elements[i];
		}

		/* offensive durability */

		out_params.offensive_durability.elements[offensive_durability_skill1] = linear_value(100.f, in_data.motherbase.pf_skill_staff.medic1_num);
		out_params.offensive_durability.elements[offensive_durability_skill2] = linear_value(200.f, in_data.motherbase.pf_skill_staff.medic2_num);
		out_params.offensive_durability.elements[offensive_durability_skill3] = linear_value(300.f, in_data.motherbase.pf_skill_staff.medic3_num);
		out_params.offensive_durability.elements[offensive_durability_unprocessed_materials] = linear_value(0.02f, unprocessed_materials);
		out_params.offensive_durability.elements[offensive_durability_processed_materials] = linear_value(0.04f, processed_materials);
		out_params.offensive_durability.elements[offensive_durability_gmp] = linear_value(0.001f, total_gmp);

		for (auto i = static_cast<std::uint32_t>(offensive_durability_rank_e); i <= offensive_durability_gmp; i++)
		{
			out_params.offensive_durability.elements[offensive_durability_sum] += out_params.offensive_durability.elements[i];
		}

		/* defensive capability */

		out_params.defensive_capability.elements[defensive_capability_total_defense] = linear_value(59.f, out_params.security_level);
		out_params.defensive_capability.elements[defensive_capability_security] = exp_value(3.f, in_data.unit_levels[game::unit_security]);
		out_params.defensive_capability.elements[defensive_capability_platforms] = linear_value(268.f, total_platforms);
		out_params.defensive_capability.elements[defensive_capability_nuclear] = linear_value(15000.f, in_data.resources[game::processed_server][game::NUCLEAR_WEAPON]);
		out_params.defensive_capability.elements[defensive_capability_total_units] = total_unit_levels(6.f, game::unit_security, game::unit_support);
		out_params.defensive_capability.elements[defensive_capability_support] = exp_value(1.5f, in_data.unit_levels[game::des_support]);
		out_params.defensive_capability.elements[defensive_capability_emplacements] = resource_capped_value(50.f, 51000u, game::ANTI_AIR_GATLING_GUN_EAST, game::ANTI_AIR_GATLING_GUN_WEST);
		out_params.defensive_capability.elements[defensive_capability_mortars] = resource_capped_value(10.f, 19000u, game::MORTAR_NORMAL, game::MORTAR_NORMAL);
		out_params.defensive_capability.elements[defensive_capability_machine_guns] = resource_capped_value(10.f, 19000u, game::EMPLACEMENT_GUN_EAST, game::EMPLACEMENT_GUN_WEST);
		out_params.defensive_capability.elements[defensive_capability_walker_gears] = resource_capped_value(71.f, 80000u, game::WALKER_GEAR_PROTO_HEUY, game::WALKER_GEAR_CFA_SUPPORT);
		out_params.defensive_capability.elements[defensive_capability_plants] = resource_value(0.13f, true, game::WORM_WOOD, game::HAOMA);
		out_params.defensive_capability.elements[defensive_capability_parasites] = resource_value(0.33f, true, game::PARASITE_FOG, game::PARASITE_CURING);
		out_params.defensive_capability.elements[defensive_capability_skill1] = linear_value(100.f, in_data.motherbase.pf_skill_staff.sentry1_num);
		out_params.defensive_capability.elements[defensive_capability_skill2] = linear_value(200.f, in_data.motherbase.pf_skill_staff.sentry2_num);
		out_params.defensive_capability.elements[defensive_capability_skill3] = linear_value(300.f, in_data.motherbase.pf_skill_staff.sentry3_num);
		out_params.defensive_capability.elements[defensive_capability_bonus] = 0;

		for (auto i = static_cast<std::uint32_t>(defensive_capability_total_defense); i <= defensive_capability_bonus; i++)
		{
			out_params.defensive_capability.elements[defensive_capability_sum] += out_params.defensive_capability.elements[i];
		}

		/* defensive durability */

		out_params.defensive_durability.elements[defensive_durability_platforms] = linear_value(892.f, total_platforms);
		out_params.defensive_durability.elements[defensive_durability_nuclear] = linear_value(25000.f, in_data.resources[game::processed_server][game::NUCLEAR_WEAPON]);
		out_params.defensive_durability.elements[defensive_durability_skill1] = linear_value(100.f, in_data.motherbase.pf_skill_staff.defender1_num);
		out_params.defensive_durability.elements[defensive_durability_skill2] = linear_value(200.f, in_data.motherbase.pf_skill_staff.defender2_num);
		out_params.defensive_durability.elements[defensive_durability_skill3] = linear_value(300.f, in_data.motherbase.pf_skill_staff.defender3_num);
		out_params.defensive_durability.elements[defensive_durability_unprocessed_materials] = linear_value(0.02f, unprocessed_materials);
		out_params.defensive_durability.elements[defensive_durability_processed_materials] = linear_value(0.02f, processed_materials);
		out_params.defensive_durability.elements[defensive_durability_gmp] = linear_value(0.001f, total_gmp);

		for (auto i = static_cast<std::uint32_t>(defensive_durability_platforms); i <= defensive_durability_gmp; i++)
		{
			out_params.defensive_durability.elements[defensive_durability_sum] += out_params.defensive_durability.elements[i];
		}
	}

	void pf_battle::update_params(player_pf_data_t& attacker_data, player_pf_data_t& defender_data)
	{
		player_pf_params_t attacker_params{};
		calculate_pf_params(attacker_data, attacker_params);

		player_pf_params_t defender_params{};
		calculate_pf_params(defender_data, defender_params);

		this->update_params(attacker_data, defender_data, attacker_params, defender_params);
	}

	void pf_battle::update_params(const player_pf_data_t& attacker_data, const player_pf_data_t& defender_data,
		const player_pf_params_t& attacker_params, const player_pf_params_t& defender_params)
	{
		this->attacker_capability_ = attacker_params.offensive_capability.elements[offensive_capability_sum] + this->get_attacker_buff() * 30000;
		this->defender_capability_ = defender_params.defensive_capability.elements[defensive_capability_sum] + this->get_defender_capability() * 30000;

		this->attacker_durability_ = attacker_params.offensive_durability.elements[offensive_durability_sum];
		this->defender_durability_ = defender_params.defensive_durability.elements[defensive_durability_sum];

		this->attacker_nuclear_ = attacker_data.resources[game::processed_server][game::NUCLEAR_WEAPON];
		this->defender_nuclear_ = defender_data.resources[game::processed_server][game::NUCLEAR_WEAPON];

		this->attacker_staff_ = attacker_data.unit_counts[game::unit_combat];
		this->defender_staff_ = defender_data.unit_counts[game::unit_security];

		this->attacker_grade_ = attacker_data.cumulative_grade;
		this->defender_security_ = defender_params.security_level;

		this->attacker_level_ = attacker_data.unit_levels[game::unit_combat];
		this->defender_level_ = defender_data.unit_levels[game::unit_security];
	}

	bool calculate_pf_params(const std::uint64_t player_id, player_pf_data_t& pf_data, player_pf_params_t& out_params)
	{
		const auto player_data = database::player_data::find(player_id);
		if (!player_data.has_value())
		{
			return false;
		}

		pf_data.local_gmp = player_data->get_local_gmp();
		pf_data.server_gmp = player_data->get_server_gmp();
		pf_data.cumulative_grade = player_data->get_cumulative_grade();

		player_data->get_motherbase(pf_data.motherbase);
		player_data->get_unit_levels(pf_data.unit_levels);
		player_data->get_unit_counts(pf_data.unit_counts);
		player_data->get_resource_arrays(pf_data.resources);
		player_data->get_staff_array(pf_data.staff);

		pf_data.fobs = database::fobs::get_fob_list(player_id);

		calculate_pf_params(pf_data, out_params);
		return true;
	}

	std::vector<pf_point_info_t> load_pf_points_table()
	{
		// https://docs.google.com/spreadsheets/d/1ktnYiA2EMzwHQW_suOmlhm4697GPXKZigmWKt-0l9io/edit?gid=1726493418#gid=1726493418
		// https://gamefaqs.gamespot.com/boards/718564-metal-gear-solid-v-the-phantom-pain/77169312

		std::vector<pf_point_info_t> table;

		auto table_j = utils::resources::load_json(RESOURCE_PF_POINTS_TABLE);
		if (!table_j.is_array())
		{
			return table;
		}

		for (auto i = 0u; i < table_j.size(); i++)
		{
			pf_point_info_t entry{};

			auto& entry_j = table_j[i];
			if (entry_j.size() >= 2)
			{
				entry.baseline = entry_j[0].get<float>();
				entry.group = entry_j[1].get<float>();
			}

			table.emplace_back(entry);
		}

		return table;
	}

	const std::vector<pf_point_info_t>& get_pf_points_table()
	{
		static const auto table = load_pf_points_table();
		return table;
	}

	std::uint32_t calculate_pf_points(const std::uint32_t grade, std::uint32_t bracket_rank)
	{
		const auto& table = get_pf_points_table();
		if (table.size() == 0)
		{
			return 0u;
		}

		const auto& entry = grade < table.size()
			? table[grade]
			: table[0];

		if (bracket_rank > 16u)
		{
			bracket_rank = 16u;
		}

		return static_cast<std::uint32_t>(entry.baseline + (static_cast<float>(16u - bracket_rank) * ((0.8f * entry.group) / 15.f)));
	}

	std::vector<std::vector<fob_events::point_exchange_param_t>> load_point_exchange_params_lists()
	{
		std::vector<std::vector<fob_events::point_exchange_param_t>> lists;

		auto lists_j = utils::resources::load_json(RESOURCE_PF_POINTS_EXCHANGE_LIST);
		for (auto i = 0u; i < lists_j.size(); i++)
		{
			lists.emplace_back(fob_events::parse_point_exchange_params(lists_j[i]));
		}

		return lists;
	}

	const std::vector<fob_events::point_exchange_param_t>& get_point_exchange_params()
	{
		static const auto lists = load_point_exchange_params_lists();
		static std::vector<fob_events::point_exchange_param_t> default_list;
		const auto league = get_current_pf_league();
		if (!league.has_value())
		{
			return default_list;
		}

		const auto idx = league->get_id() % lists.size();
		return lists[idx];
	}

	namespace impl
	{
		template <database_type_t Type>
		std::optional<pf_league> get_current_pf_league1(database_t& db)
		{
			auto results = db.get_database<Type>()->operator()(
				sqlpp::select(sqlpp::all_of(pf_league::table))
						.from(pf_league::table)
							.where(pf_league::table.start_date <= std::chrono::system_clock::now() &&
								   pf_league::table.end_date > std::chrono::system_clock::now()).limit(1u));

			if (results.empty())
			{
				return {};
			}

			return pf_league(results.front());
		}

		template <database_type_t Type>
		std::optional<pf_league> get_current_pf_league2()
		{
			return database::access<std::optional<pf_league>>([](database_t& db)
			{
				return impl::get_current_pf_league1<Type>(db);
			});
		}

		template <database_type_t Type>
		std::vector<pf_league> get_past_leagues_of_state(database_t& db, const std::uint32_t state)
		{
			auto results = db.get_database<Type>()->operator()(
				sqlpp::select(sqlpp::all_of(pf_league::table))
						.from(pf_league::table)
							.where(pf_league::table.end_date < std::chrono::system_clock::now() && 
								   pf_league::table.state == state));

			std::vector<pf_league> list;

			for (auto& row : results)
			{
				list.emplace_back(row);
			}

			return list;
		}

		template <database_type_t Type>
		bool create_pf_league(database_t& db, const std::chrono::system_clock::time_point& start_date, const std::chrono::system_clock::time_point& end_date)
		{
			auto result = db.get_database<Type>()->operator()(
				sqlpp::insert_into(pf_league::table)
					.set(pf_league::table.start_date = start_date,
						 pf_league::table.end_date = end_date,
						 pf_league::table.state = static_cast<std::uint32_t>(league_state_none)));
			return result != 0ull;
		}

		template <database_type_t Type>
		void set_league_state(database_t& db, const std::uint64_t league_id, const std::uint32_t state)
		{
			db.get_database<Type>()->operator()(
				sqlpp::update(pf_league::table)
					.set(pf_league::table.state = state)
							.where(pf_league::table.id == league_id)
				);
		}
		
		template <database_type_t Type>
		void set_bracket_state(database_t& db, const std::uint64_t bracket_id, const std::uint32_t state)
		{
			db.get_database<Type>()->operator()(
				sqlpp::update(pf_bracket::table)
					.set(pf_bracket::table.state = state)
							.where(pf_bracket::table.id == bracket_id)
				);
		}

		template <database_type_t Type>
		std::uint64_t create_pf_bracket(database_t& db, const std::uint64_t league_id)
		{
			return db.get_database<Type>()->operator()(
				sqlpp::insert_into(pf_bracket::table)
					.set(pf_bracket::table.league_id = league_id));
		}
		
		template <database_type_t Type>
		std::uint64_t create_pf_competitor(database_t& db, const std::uint64_t league_id, const std::uint64_t bracket_id, const std::uint64_t player_id)
		{
			return db.get_database<Type>()->operator()(
				sqlpp::insert_into(pf_competitor::table)
					.set(pf_competitor::table.league_id = league_id,
						 pf_competitor::table.bracket_id = bracket_id,
						 pf_competitor::table.player_id = player_id));
		}

				
		template <database_type_t Type>
		std::uint64_t create_pf_battle(database_t& db, const std::uint64_t league_id, const std::uint64_t bracket_id, 
			const std::uint32_t section,
			const std::uint64_t attacker_id, const std::uint64_t defender_id, const std::chrono::system_clock::time_point date)
		{
			return db.get_database<Type>()->operator()(
				sqlpp::insert_into(pf_battle::table)
					.set(pf_battle::table.league_id = league_id,
						 pf_battle::table.bracket_id = bracket_id,
						 pf_battle::table.section = section,
						 pf_battle::table.attacker_id = attacker_id,
						 pf_battle::table.defender_id = defender_id,
						 pf_battle::table.date = date
					));
		}

		template <database_type_t Type>
		std::vector<player_records::player_record> find_unmatched_players(database_t& db, const std::uint64_t league_id, const std::uint32_t limit)
		{
			const auto matched_members = 
				sqlpp::select(pf_competitor::table.player_id)
					.from(pf_competitor::table).where(pf_competitor::table.league_id == league_id);

			auto results = db.get_database<Type>()->operator()(
				sqlpp::select(
					sqlpp::all_of(player_records::player_record::table))
						.from(player_records::player_record::table)
							.where(!IS_SYSTEM_PLAYER_ID(player_records::player_record::table.player_id) && 
								   !player_records::player_record::table.player_id.in(matched_members) && player_records::player_record::table.has_fob)
								.order_by(player_records::player_record::table.league_grade.desc())
									.limit(limit));

			std::vector<player_records::player_record> list;
			for (auto& row : results)
			{
				list.emplace_back(row);
			}

			return list;
		}

		template <database_type_t Type>
		std::vector<pf_battle> get_battles_to_run(database_t& db, const std::uint64_t league_id, const std::uint32_t limit)
		{
			auto results = db.get_database<Type>()->operator()(
				sqlpp::select(
					sqlpp::all_of(pf_battle::table))
						.from(pf_battle::table)
							.where(pf_battle::table.winner_state == static_cast<std::uint32_t>(battle_winner_none) && 
								   pf_battle::table.league_id == league_id && pf_battle::table.date <= std::chrono::system_clock::now())
									.order_by(pf_battle::table.date.asc()).limit(limit));

			std::vector<pf_battle> list;
			for (auto& row : results)
			{
				list.emplace_back(row);
			}

			return list;
		}
		
		template <database_type_t Type>
		std::uint64_t get_num_battles_left(database_t& db, const std::uint64_t league_id)
		{
			auto results = db.get_database<Type>()->operator()(
				sqlpp::select(sqlpp::count(1))
						.from(pf_battle::table)
							.where(pf_battle::table.winner_state == static_cast<std::uint32_t>(battle_winner_none) && 
								   pf_battle::table.league_id == league_id));

			return results.front().count.value();
		}
		
		template <database_type_t Type>
		void update_pf_battle_params(database_t& db, const pf_battle& battle)
		{
			db.get_database<Type>()->operator()(
				sqlpp::update(pf_battle::table)
					.set(pf_battle::table.attacker_capability = battle.get_attacker_capability(),
						 pf_battle::table.defender_capability = battle.get_defender_capability(),
						 pf_battle::table.attacker_durability = battle.get_attacker_durability(),
						 pf_battle::table.defender_durability = battle.get_defender_durability(),
						 pf_battle::table.attacker_level = battle.get_attacker_level(),
						 pf_battle::table.defender_level = battle.get_defender_level(),
						 pf_battle::table.attacker_grade = battle.get_attacker_grade(),
						 pf_battle::table.defender_security = battle.get_defender_security(),
						 pf_battle::table.attacker_staff = battle.get_attacker_staff(),
						 pf_battle::table.defender_staff = battle.get_defender_staff(),
						 pf_battle::table.attacker_nuclear = battle.get_attacker_nuclear(),
						 pf_battle::table.defender_nuclear = battle.get_defender_nuclear())
							.where(pf_battle::table.id == battle.get_id())
				);
		}

		template <database_type_t Type>
		void set_pf_battle_result(database_t& db, const std::uint64_t battle_id, const battle_result_t& result)
		{
			db.get_database<Type>()->operator()(
				sqlpp::update(pf_battle::table)
					.set(pf_battle::table.winner_state = static_cast<std::uint32_t>(result.state),
						 pf_battle::table.attacker_points = result.attacker_points,
						 pf_battle::table.defender_points = result.defender_points)
							.where(pf_battle::table.id == battle_id)
				);
		}
		
		template <database_type_t Type>
		void add_battle_stats(database_t& db, const std::uint64_t bracket_id, const std::uint64_t player_id, const battle_result_stats_t& stats)
		{
			db.get_database<Type>()->operator()(
				sqlpp::update(pf_competitor::table)
					.set(pf_competitor::table.victory_points = pf_competitor::table.victory_points + stats.victory_points,
						 pf_competitor::table.win = pf_competitor::table.win + stats.win,
						 pf_competitor::table.narrow_win = pf_competitor::table.narrow_win + stats.narrow_win,
						 pf_competitor::table.attack_win = pf_competitor::table.attack_win + stats.attack_win,
						 pf_competitor::table.defense_win = pf_competitor::table.defense_win + stats.defense_win,
						 pf_competitor::table.lose = pf_competitor::table.lose + stats.lose,
						 pf_competitor::table.narrow_lose = pf_competitor::table.narrow_lose + stats.narrow_lose,
						 pf_competitor::table.attack_lose = pf_competitor::table.attack_lose + stats.attack_lose,
						 pf_competitor::table.defense_lose = pf_competitor::table.defense_lose + stats.defense_lose)
							.where(pf_competitor::table.bracket_id == bracket_id && pf_competitor::table.player_id == player_id)
				);
		}

		template <database_type_t Type>
		std::optional<pf_competitor> get_player_competitor_instance(const std::uint64_t league_id, const std::uint64_t player_id)
		{
			return database::access<std::optional<pf_competitor>>([&](database_t& db)
				-> std::optional<pf_competitor>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(pf_competitor::table))
							.from(pf_competitor::table)
								.where(pf_competitor::table.player_id == player_id && 
									   pf_competitor::table.league_id == league_id));

				if (results.empty())
				{
					return {};
				}

				return pf_competitor(results.front());
			});
		}

		template <database_type_t Type>
		std::vector<pf_bracket> get_active_brackets(const std::uint64_t league_id, const std::uint32_t limit)
		{
			return database::access<std::vector<pf_bracket>>([&](database_t& db)
				-> std::vector<pf_bracket>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(pf_bracket::table))
							.from(pf_bracket::table)
								.where(pf_bracket::table.league_id == league_id && pf_bracket::table.state != static_cast<std::uint32_t>(bracket_state_done))
									.limit(limit));

				std::vector<pf_bracket> list;

				for (auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::vector<pf_competitor> get_players_in_bracket(const std::uint64_t bracket_id)
		{
			return database::access<std::vector<pf_competitor>>([&](database_t& db)
				-> std::vector<pf_competitor>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(pf_competitor::table))
							.from(pf_competitor::table)
								.where(pf_competitor::table.bracket_id == bracket_id)
									.order_by(pf_competitor::table.victory_points.desc()));

				std::vector<pf_competitor> list;

				for (auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		std::vector<pf_battle> get_player_battles(const std::uint64_t bracket_id, const std::uint64_t player_id)
		{
			return database::access<std::vector<pf_battle>>([&](database_t& db)
				-> std::vector<pf_battle>
			{
				auto results = db.get_database<Type>()->operator()(
					sqlpp::select(
						sqlpp::all_of(pf_battle::table))
							.from(pf_battle::table)
								.where(pf_battle::table.bracket_id == bracket_id && 
									  (pf_battle::table.attacker_id == player_id || pf_battle::table.defender_id == player_id))
									.order_by(pf_battle::table.date.asc(), (pf_battle::table.attacker_id == player_id).desc()));

				std::vector<pf_battle> list;

				for (auto& row : results)
				{
					list.emplace_back(row);
				}

				return list;
			});
		}

		template <database_type_t Type>
		void inc_battle_buff(const std::uint64_t battle_id, const std::uint32_t attacker_buff, const std::uint32_t defender_buff, const bool inc)
		{
			database::access([&](database_t& db)
			{
				if (inc)
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(pf_battle::table)
							.set(pf_battle::table.attacker_buff = pf_battle::table.attacker_buff + attacker_buff,
								 pf_battle::table.defender_buff = pf_battle::table.defender_buff + defender_buff)
									.where(pf_battle::table.id == battle_id)
						);
				}
				else
				{
					db.get_database<Type>()->operator()(
						sqlpp::update(pf_battle::table)
							.set(pf_battle::table.attacker_buff = pf_battle::table.attacker_buff - attacker_buff,
								 pf_battle::table.defender_buff = pf_battle::table.defender_buff - defender_buff)
									.where(pf_battle::table.id == battle_id && 
										   pf_battle::table.attacker_buff >= attacker_buff && 
										   pf_battle::table.defender_buff >= defender_buff)
						);
				}

			});
		}

		template <database_type_t Type>
		void clear_pf_league(database_t& db, const std::uint64_t league_id)
		{
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_battle::table).where(pf_battle::table.league_id == league_id));
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_competitor::table).where(pf_competitor::table.league_id == league_id));
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_bracket::table).where(pf_bracket::table.league_id == league_id));
		}

		template <database_type_t Type>
		void delete_all_pf_leagues(database_t& db)
		{
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_battle::table).unconditionally());
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_competitor::table).unconditionally());
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_bracket::table).unconditionally());
			db.get_database<Type>()->operator()(sqlpp::remove_from(pf_league::table).unconditionally());
		}
	}

	std::optional<pf_league> get_current_pf_league(database_t& db)
	{
		RUN_IMPL(impl::get_current_pf_league1, db);
	}

	std::optional<pf_league> get_current_pf_league()
	{
		RUN_IMPL(impl::get_current_pf_league2);
	}

	std::vector<pf_league> get_past_leagues_of_state(database_t& db, const std::uint32_t state)
	{
		RUN_IMPL(impl::get_past_leagues_of_state, db, state);
	}

	bool create_pf_league(database_t& db, const std::chrono::system_clock::time_point& start_date, const std::chrono::system_clock::time_point& end_date)
	{
		RUN_IMPL(impl::create_pf_league, db, start_date, end_date);
	}

	void set_league_state(database_t& db, const std::uint64_t league_id, const std::uint32_t state)
	{
		RUN_IMPL(impl::set_league_state, db, league_id, state);
	}

	void set_bracket_state(database_t& db, const std::uint64_t bracket_id, const std::uint32_t state)
	{
		RUN_IMPL(impl::set_bracket_state, db, bracket_id, state);
	}

	std::uint64_t create_pf_bracket(database_t& db, const std::uint64_t league_id)
	{
		RUN_IMPL(impl::create_pf_bracket, db, league_id);
	}

	std::uint64_t create_pf_competitor(database_t& db, const std::uint64_t league_id, const std::uint64_t bracket_id, const std::uint64_t player_id)
	{
		RUN_IMPL(impl::create_pf_competitor, db, league_id, bracket_id, player_id);
	}

	std::uint64_t create_pf_battle(database_t& db, const std::uint64_t league_id, const std::uint64_t bracket_id, const std::uint32_t section,
		const std::uint64_t attacker_id, const std::uint64_t defender_id, const std::chrono::system_clock::time_point date)
	{
		RUN_IMPL(impl::create_pf_battle, db, league_id, bracket_id, section, attacker_id, defender_id, date);
	}

	std::vector<pf_battle> get_battles_to_run(database_t& db, const std::uint64_t league_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_battles_to_run, db, league_id, limit);
	}

	std::uint64_t get_num_battles_left(database_t& db, const std::uint64_t league_id)
	{
		RUN_IMPL(impl::get_num_battles_left, db, league_id);
	}

	std::vector<player_records::player_record> find_unmatched_players(database_t& db, const std::uint64_t league_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::find_unmatched_players, db, league_id, limit);
	}

	void set_pf_battle_result(database_t& db, const std::uint64_t battle_id, const battle_result_t& result)
	{
		RUN_IMPL(impl::set_pf_battle_result, db, battle_id, result);
	}

	void update_pf_battle_params(database_t& db, const pf_battle& battle)
	{
		RUN_IMPL(impl::update_pf_battle_params, db, battle);
	}

	void add_battle_stats(database_t& db, const std::uint64_t bracket_id, const std::uint64_t player_id, const battle_result_stats_t& stats)
	{
		RUN_IMPL(impl::add_battle_stats, db, bracket_id, player_id, stats);
	}

	std::optional<pf_competitor> get_player_competitor_instance(const std::uint64_t league_id, const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_player_competitor_instance, league_id, player_id);
	}

	std::vector<pf_bracket> get_active_brackets(const std::uint64_t league_id, const std::uint32_t limit)
	{
		RUN_IMPL(impl::get_active_brackets, league_id, limit);
	}

	std::vector<pf_competitor> get_players_in_bracket(const std::uint64_t bracket_id)
	{
		RUN_IMPL(impl::get_players_in_bracket, bracket_id);
	}

	std::vector<pf_battle> get_player_battles(const std::uint64_t bracket_id, const std::uint64_t player_id)
	{
		RUN_IMPL(impl::get_player_battles, bracket_id, player_id);
	}

	void delete_all_pf_leagues(database_t& db)
	{
		RUN_IMPL(impl::delete_all_pf_leagues, db);
	}

	void clear_pf_league(database_t& db, const std::uint64_t league_id)
	{
		RUN_IMPL(impl::clear_pf_league, db, league_id);
	}

	void inc_battle_buff(const std::uint64_t battle_id, const std::uint32_t attacker_buff, const std::uint32_t defender_buff, const bool inc)
	{
		RUN_IMPL(impl::inc_battle_buff, battle_id, attacker_buff, defender_buff, inc);
	}

	bool create_pf_battles(database_t& db, const pf_league& league)
	{
		const auto players = find_unmatched_players(db, league.get_id(), pf_bracket_size);
		if (players.empty())
		{
			return false;
		}

		const auto bracket_id = create_pf_bracket(db, league.get_id());

		for (const auto& player : players)
		{
			create_pf_competitor(db, league.get_id(), bracket_id, player.get_player_id());
		}

		// ai code below, sorry i could not do it lol

		const auto num_players = static_cast<std::uint32_t>(players.size());
		const bool is_odd = (num_players % 2 != 0);
		const auto total_sections = is_odd ? num_players : (num_players - 1);
		const auto total_virtual_players = is_odd ? (num_players + 1) : num_players;

		std::chrono::system_clock::time_point base_start(league.get_start_date());
		base_start += 6h;

		std::vector<std::chrono::system_clock::time_point> section_dates(total_sections);

		constexpr auto duration = 6 * 24h;
		const auto interval = (total_sections > 1)
			? std::chrono::duration_cast<std::chrono::hours>(duration / (total_sections - 1))
			: 0h;

		for (auto i = 0u; i < total_sections; i++)
		{
			section_dates[i] = base_start + (i * interval);
		}

		auto section_idx = 0u;
		for (auto slot = 0u; slot < total_sections; slot++)
		{
			const auto& slot_date = section_dates[slot];
			for (auto i = 0u; i < total_virtual_players / 2; i++)
			{
				auto p1_idx = (slot + i) % (total_virtual_players - 1);
				auto p2_idx = (slot + total_virtual_players - 1 - i) % (total_virtual_players - 1);

				if (i == 0)
				{
					p1_idx = total_virtual_players - 1;
				}

				if (is_odd && (p1_idx == num_players || p2_idx == num_players))
				{
					continue;
				}

				auto idx = section_idx++;
				create_pf_battle(db, league.get_id(), bracket_id, idx, players[p1_idx].get_player_id(), players[p2_idx].get_player_id(), slot_date);
				create_pf_battle(db, league.get_id(), bracket_id, idx, players[p2_idx].get_player_id(), players[p1_idx].get_player_id(), slot_date);
			}
		}

		return true;
	}

	void run_pf_battle(const pf_battle& battle, const player_pf_params_t& attacker, const player_pf_params_t& defender, battle_result_t& result)
	{
		// https://gamefaqs.gamespot.com/boards/718564-metal-gear-solid-v-the-phantom-pain/73549233

		result.attacker_points = 0;
		result.defender_points = 0;

		auto attacker_capability = static_cast<float>(attacker.offensive_capability.elements[offensive_capability_sum]);
		auto attacker_durability = static_cast<float>(attacker.offensive_durability.elements[offensive_durability_sum]);

		attacker_capability += static_cast<float>(battle.get_attacker_buff()) * 30000;

		auto defender_capability = static_cast<float>(defender.defensive_capability.elements[defensive_capability_sum]);
		auto defender_durability = static_cast<float>(defender.defensive_durability.elements[defensive_durability_sum]);

		defender_capability += static_cast<float>(battle.get_defender_buff()) * 30000;

		auto attacker_hp = attacker_durability;
		auto defender_hp = defender_durability;

		auto is_attack = true;
		auto attacker_attacks = 0;
		auto defender_attacks = 0;

		const auto run_turn = [&](bool attack)
		{
			if (attack)
			{
				++attacker_attacks;
				defender_hp -= attacker_capability;
			}
			else
			{
				++defender_attacks;
				attacker_hp -= defender_capability;
			}

		};

		while (attacker_hp > 0 && defender_hp > 0)
		{
			run_turn(is_attack);
			is_attack = !is_attack;
		}

		const auto calculate_points = [](const float winner_power, const float winner_health, const float loser_power, const std::int32_t loser_attacks)
		{
			const auto base = std::ceil((winner_power + winner_health - loser_power * static_cast<float>(loser_attacks)) / 100);
			const auto bonus = loser_power > winner_power
				? std::ceil((loser_power - winner_power) * 2.2f)
				: 0;
			return static_cast<std::int32_t>(std::ceil(base + bonus));
		};

		if (attacker_hp > 0)
		{
			run_turn(false);

			if (attacker_hp < 0)
			{
				result.state = battle_winner_draw;

				const auto diff = std::abs(attacker_capability - defender_capability);
				const auto draw_points = static_cast<std::int32_t>(std::ceil(diff / 272.72f));

				result.attacker_points = draw_points;
				result.defender_points = draw_points;
			}
			else
			{
				run_turn(false);
				result.state = battle_winner_attacker;
				result.attacker_points = calculate_points(attacker_capability, attacker_durability, defender_capability, defender_attacks);
			}
		}
		else if (defender_hp > 0)
		{
			run_turn(true);
			result.state = battle_winner_defender;
			result.defender_points = calculate_points(defender_capability, defender_durability, attacker_capability, attacker_attacks);
		}
		else
		{
			result.attacker_points = 0;
			result.defender_points = 0;
			result.state = battle_winner_draw;
		}
	}

	bool run_pf_battles(database_t& db, const pf_league& league)
	{
		struct cache_entry_t
		{
			player_pf_data_t data;
			player_pf_params_t params;
		};

		std::unordered_map<std::uint64_t, cache_entry_t> cached_params;

		const auto get_player_params = [&](const std::uint64_t player_id)
			-> cache_entry_t&
		{
			const auto iter = cached_params.find(player_id);
			if (iter != cached_params.end())
			{
				return iter->second;
			}

			auto& params = cached_params[player_id];
			calculate_pf_params(player_id, params.data, params.params);
			return params;
		};

		auto battles = get_battles_to_run(db, league.get_id(), 64u);
		for (auto& battle : battles)
		{
			auto& attacker_cache = get_player_params(battle.get_attacker_id());
			auto& defender_cache = get_player_params(battle.get_defender_id());

			battle_result_t result{};
			battle.update_params(attacker_cache.data, defender_cache.data, attacker_cache.params, defender_cache.params);
			update_pf_battle_params(db, battle);

			run_pf_battle(battle, attacker_cache.params, defender_cache.params, result);
			set_pf_battle_result(db, battle.get_id(), result);

			const auto is_defense_lose = result.state == battle_winner_attacker ? 1u : 0u;
			const auto is_attacker_lose = result.state == battle_winner_defender ? 1u : 0u;
			const auto is_attacker_win = is_defense_lose;
			const auto is_defender_win = is_attacker_lose;

			battle_result_stats_t attacker_stats{};
			attacker_stats.victory_points = result.attacker_points;

			attacker_stats.win = is_attacker_win;
			attacker_stats.narrow_win = 0u;
			attacker_stats.attack_win = is_attacker_win;
			attacker_stats.defense_win = 0u;

			attacker_stats.lose = is_attacker_lose;
			attacker_stats.narrow_lose = 0u;
			attacker_stats.attack_lose = is_attacker_lose;
			attacker_stats.defense_lose = 0u;

			add_battle_stats(db, battle.get_bracket_id(), battle.get_attacker_id(), attacker_stats);

			battle_result_stats_t defender_stats{};
			defender_stats.victory_points = result.defender_points;

			defender_stats.win = is_defender_win;
			defender_stats.narrow_win = 0u;
			defender_stats.attack_win = 0u;
			defender_stats.defense_win = is_defender_win;

			defender_stats.lose = is_defense_lose;
			defender_stats.narrow_lose = 0u;
			defender_stats.attack_lose = 0u;
			defender_stats.defense_lose = is_defense_lose;

			add_battle_stats(db, battle.get_bracket_id(), battle.get_defender_id(), defender_stats);

			if (is_attacker_win)
			{
				event_rankings::increment_event_value(battle.get_attacker_id(), event_rankings::league_wins, 1);
			}
			else if (is_defender_win)
			{
				event_rankings::increment_event_value(battle.get_defender_id(), event_rankings::league_wins, 1);
			}
		}

		const auto battles_left = get_num_battles_left(db, league.get_id());
		return battles_left > 0;
	}

	void update_player_grades(const std::vector<pf_competitor>& players)
	{
		auto top_3_threshold = 3u;
		switch (players.size())
		{
		case 2:
		case 3:
		case 4:
			top_3_threshold = 1u;
			break;
		case 5:
		case 6:
			top_3_threshold = 2u;
			break;
		default:
			top_3_threshold = 3u;
			break;
		}

		for (auto i = 0u; i < players.size(); i++)
		{
			const auto player_record = database::player_records::find(players[i].get_player_id());
			if (player_record.has_value())
			{
				const auto pf_points = calculate_pf_points(player_record->get_league_grade(), i);
				database::player_records::add_pf_points(player_record->get_player_id(), pf_points);
			}

			if (i < top_3_threshold)
			{
				database::player_records::inc_league_grade(players[i].get_player_id(), true);
			}
			else if (i >= players.size() - top_3_threshold)
			{
				database::player_records::inc_league_grade(players[i].get_player_id(), false);
			}
		}
	}
	
	bool update_league_ranks(database_t& db, const pf_league& league)
	{
		const auto brackets = get_active_brackets(league.get_id(), 100u);
		if (brackets.empty())
		{
			return false;
		}

		for (const auto& bracket : brackets)
		{
			const auto players = get_players_in_bracket(bracket.get_id());
			if (players.size() > 1)
			{
				update_player_grades(players);
			}

			for (auto& player : players)
			{
				database::player_records::add_league_points(player.get_player_id(), player.get_victory_points());
			}

			set_bracket_state(db, bracket.get_id(), bracket_state_done);
		}
		
		database::player_records::update_league_ranking();

		return true;
	}

	void update_past_leagues(database_t& db)
	{
		const auto completed_leagues = get_past_leagues_of_state(db, league_state_completed);
		for (const auto& league : completed_leagues)
		{
			if (!update_league_ranks(db, league))
			{
				set_league_state(db, league.get_id(), league_state_destroy);
			}
		}

		const auto destroy_leagues = get_past_leagues_of_state(db, league_state_destroy);
		for (const auto& league : destroy_leagues)
		{
			clear_pf_league(db, league.get_id());
			set_league_state(db, league.get_id(), league_state_dead);
		}
	}

	void update_league(database_t& db)
	{
		if (!database::vars.run_pf_league)
		{
			return;
		}

		update_past_leagues(db);

		const auto league = get_current_pf_league(db);
		if (!league.has_value())
		{
			std::chrono::system_clock::time_point start;
			std::chrono::system_clock::time_point end;
			fob_events::get_maintenance_range(start, end);
			create_pf_league(db, start, end);
			return;
		}

		switch (league->get_state())
		{
		case league_state_none:
		{
			set_league_state(db, league->get_id(), league_state_initial);
			return;
		}
		case league_state_initial:
		{
			if (!create_pf_battles(db, league.value()))
			{
				set_league_state(db, league->get_id(), league_state_running);
			}

			return;
		}
		case league_state_running:
		{
			if (!run_pf_battles(db, league.value()))
			{
				set_league_state(db, league->get_id(), league_state_completed);
			}
			return;
		}
		case league_state_completed:
		{
			return;
		}
		case league_state_destroy:
		{
			return;
		}
		}
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.pf_leagues.create");
			database.run_query("mgstpp.pf_brackets.create");
			database.run_query("mgstpp.pf_competitors.create");
			database.run_query("mgstpp.pf_battles.create");
#ifdef DEBUG
			delete_all_pf_leagues(database);
#endif
		}

		void run_tasks(database_t& database) override
		{
			update_league(database);
		}
	};
}

REGISTER_TABLE(database::pf_league::table, -1)
