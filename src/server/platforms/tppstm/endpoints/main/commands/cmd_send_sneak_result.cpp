#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_events.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/event_rankings.hpp"

namespace emulator::tpp
{
	namespace
	{
		struct soldier_param
		{
			std::uint32_t param_1;
			std::uint32_t seed;
		};

		enum soldier_array_action
		{
			soldier_kill = 0,
			soldier_injure = 1,
			soldier_capture = 2
		};

		std::unordered_set<std::uint32_t> parse_soldier_id_list(nlohmann::json& list)
		{
			if (!list.is_array())
			{
				return {};
			}

			std::unordered_set<std::uint32_t> soldier_ids;

			for (auto i = 0ull; i < list.size(); i++)
			{
				auto& soldier_param = list[i]["param"];
				if (soldier_param.size() != 2 || !soldier_param[0].is_number_unsigned() || !soldier_param[1].is_number_unsigned())
				{
					continue;
				}

				//const auto param_1 = soldier_param[0].get<std::uint32_t>();
				const auto seed = soldier_param[1].get<std::uint32_t>();
				soldier_ids.insert(seed);
			}

			return soldier_ids;
		}

		void modify_staff_array(database::player_data::staff_array_container& staff_array, 
			std::unordered_set<std::uint32_t>& soldier_ids, const soldier_array_action action)
		{
			for (auto i = 0u; i < database::player_data::max_staff_count; i++)
			{
				auto staff = &staff_array[i];
				const auto iter = soldier_ids.find(staff->fields.seed.data);
				if (iter == soldier_ids.end())
				{
					continue;
				}

				soldier_ids.erase(iter);

				switch (action)
				{
				case soldier_injure:
					staff->fields.status_sync.health_state = 1;
					staff->fields.status_sync.designation = database::player_data::des_sickbay;
					break;
				case soldier_capture: // todo: implement fob prison list
				case soldier_kill:
					std::memset(staff, 0, sizeof(database::player_data::staff_t));
					break;
				}
			}
		}

		void update_staff(
			const std::optional<database::players::player>& attacker, 
			const std::optional<database::player_data::player_data>& attacker_data,
			const std::optional<database::players::player>& owner,
			nlohmann::json& data)
		{
			auto owner_data = database::player_data::find(owner->get_id());
			auto& new_staff_array = owner_data->get_staff_array();

			static std::vector<std::pair<std::string, soldier_array_action>> soldier_list_map =
			{
				{"capture_soldier_id", soldier_capture},
				{"injure_soldier_id", soldier_injure},
				{"kill_soldier_id", soldier_kill},
			};

			for (const auto& [name, action] : soldier_list_map)
			{
				auto list = parse_soldier_id_list(data[name]);
				modify_staff_array(new_staff_array, list, action);
			}

			database::player_data::unit_counts_t counts{};
			database::player_data::unit_levels_t levels{};

			auto new_staff_count = 0;
			for (auto i = 0u; i < database::player_data::max_staff_count; i++)
			{
				const auto staff = &new_staff_array[i];
				if (staff->fields.status_sync.designation != 0)
				{
					new_staff_count++;
					
					if (staff->fields.status_sync.designation >= database::player_data::des_units_start &&
						staff->fields.status_sync.designation < database::player_data::des_units_end)
					{
						counts[staff->fields.status_sync.designation - database::player_data::des_units_start]++;
					}
				}
			}

			for (auto i = 0; i < database::player_data::unit_count; i++)
			{
				levels[i] = owner_data->get_unit_level(i);
			}

			database::player_data::set_soldier_data(owner->get_id(), new_staff_count, new_staff_array, levels, counts);
		}
	}

	nlohmann::json cmd_send_sneak_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}
		
		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto _0 = gsl::finally([&]
		{
			database::players::abort_mother_base(player->get_id());
		});

		const auto& sneak_result_j = data["sneak_result"];
		const auto& sneak_point_j = data["sneak_point"];
		const auto& event_point_j = data["event_point"];
		const auto& is_event_j = data["is_event"];
		const auto& mode_str_j = data["mode"];
		const auto& mother_base_id_j = data["mother_base_id"];

		if (!sneak_result_j.is_string() || !sneak_point_j.is_number() || !event_point_j.is_number_unsigned() ||
			!is_event_j.is_number_unsigned() || !mode_str_j.is_string() || !mother_base_id_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto sneak_result = sneak_result_j.get<std::string>();
		const auto is_win = sneak_result == "WIN";
		const auto sneak_point = data["sneak_point"].get<std::int32_t>();
		const auto event_point = data["event_point"].get<std::uint32_t>();
		const auto is_event = data["is_event"].get<std::uint32_t>() == 1;
		const auto mode_str = data["mode"].get<std::string>();
		const auto mother_base_id = data["mother_base_id"].get<std::uint64_t>();

		const auto fob = database::fobs::get_fob(mother_base_id);
		if (!fob.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto owner = database::players::find(fob->get_player_id());
		if (!owner.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto mode = database::players::get_sneak_mode_id(mode_str);
		if (mode == database::players::mode_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		const auto send_update = [&]
		{
			const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());
			if (!active_sneak.has_value())
			{
				return ERR_DATABASE;
			}

			if (active_sneak->get_mode() != mode)
			{
				return ERR_DATABASE;
			}

			if (active_sneak->get_mode() == database::players::mode_actual)
			{
				nlohmann::json sneak_data = data;
				sneak_data.erase("msgid");
				sneak_data.erase("rqid");

				sneak_data["event"]["attacker_info"] = player_info(player);

				const auto is_sneak = active_sneak->is_sneak();
				database::player_records::add_sneak_result(player->get_id(), fob->get_player_id(), sneak_point, is_win, is_sneak);

				database::event_rankings::set_value_if_bigger(player->get_id(), database::event_rankings::most_ep_in_mission, sneak_point);

				if (is_sneak) // attack
				{
					if (!active_sneak->is_security_challenge())
					{
						database::player_records::set_shield_date(active_sneak->get_owner_id(), is_win);
					}

					auto deploy_damage_opt = player_data->get_fob_deploy_damage_param();
					if (deploy_damage_opt.has_value())
					{
						auto& deploy_damage = deploy_damage_opt.value();
						if (deploy_damage["motherbase_id"] == fob->get_id())
						{
							database::player_data::set_fob_deploy_damage_param(player->get_id(), {});
						}
					}

					if (is_win)
					{
						const auto defense_sneak = database::players::find_active_sneak(active_sneak->get_owner_id(), false, true);
						const auto event_id = defense_sneak.has_value()
							? database::event_rankings::cores_reached_defender
							: database::event_rankings::cores_reached_no_defender;
						database::event_rankings::increment_event_value(player->get_id(), event_id, 1);
					}

					update_staff(player, player_data, owner, data);

					auto& active_sneak_val = active_sneak.value();
					if (!database::sneak_results::add_sneak_result(player.value(), fob.value(), active_sneak_val, is_win, sneak_data))
					{
						result["result"] = utils::tpp::get_error(ERR_DATABASE);
					}
				}
				else // defense
				{
					const auto& count_of_neutralized_by_intruder_j = data["count_of_neutralized_by_intruder"];
					if (count_of_neutralized_by_intruder_j.is_number_unsigned())
					{
						const auto c = std::min(100u, count_of_neutralized_by_intruder_j.get<std::uint32_t>());
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::times_neutralized_by_intruder, c);
					}

					const auto& count_of_neutralize_intruder_j = data["count_of_neutralize_intruder"];
					if (count_of_neutralize_intruder_j.is_number_unsigned() && player->get_id() == active_sneak->get_owner_id())
					{
						const auto c = std::min(100u, count_of_neutralize_intruder_j.get<std::uint32_t>());
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::defender_neutralized_intruder, c);
					}

					if (is_win)
					{
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::successful_defenses, 1);
					}

					if (active_sneak->get_owner_id() == player->get_id())
					{
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::defense_deployments, 1);
					}
					else
					{
						database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::defense_deployments_support, 1);
						database::event_rankings::increment_event_value(active_sneak->get_owner_id(), database::event_rankings::defense_deployments_support_received, 1);
					}
				}
			}

			return NOERR;
		};

		if (is_event && database::fob_events::is_event_player(owner->get_id()))
		{
			database::player_records::add_event_points(player->get_id(), event_point);
			database::event_rankings::set_value_if_bigger(player->get_id(), database::event_rankings::most_ep_in_mission, sneak_point);
			database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::fob_event_ranking, event_point);
			database::event_rankings::increment_event_value(player->get_id(), database::event_rankings::cores_reached_no_defender, 1);
			database::player_records::add_sneak_result(player->get_id(), fob->get_player_id(), sneak_point, is_win, true);
		}

		if (owner->is_real_player())
		{
			const auto err = send_update();
			if (err != NOERR)
			{
				return error(err);
			}
		}

		const auto new_stats = database::player_records::find(player->get_id());
		if (!new_stats.has_value())
		{
			return error(ERR_DATABASE);
		}

		result["sneak_point"] = new_stats->get_fob_point();
		result["is_security_challenge"] = 0;
		result["is_wormhole_open"] = 0;
		result["event_point"] = new_stats->get_event_point();

		return result;
	}
}
