#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/event_rankings.hpp"

#include <utils/nt.hpp>

namespace tpp
{
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
		if (!player_data.get())
		{
			return error(ERR_DATABASE);
		}

		const auto _0 = gsl::finally([&]
		{
			database::players::abort_mother_base(player->get_id());
		});

		try
		{
			const auto sneak_result = data["sneak_result"].get<std::string>();
			const auto is_win = sneak_result == "WIN";
			const auto sneak_point = data["sneak_point"].get<std::int32_t>();
			const auto mode_str = data["mode"].get<std::string>();
			const auto mother_base_id = data["mother_base_id"].get<std::uint64_t>();

			const auto fob = database::fobs::get_fob(mother_base_id);
			if (!fob.has_value())
			{
				return error(ERR_DATABASE);
			}

			const auto mode = database::players::get_sneak_mode_id(mode_str);
			if (mode == database::players::mode_invalid)
			{
				return error(ERR_INVALIDARG);
			}

			const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());
			if (!active_sneak.has_value())
			{
				return error(ERR_DATABASE);
			}

			if (active_sneak->get_mode() != mode)
			{
				return error(ERR_DATABASE);
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

			const auto new_stats = database::player_records::find(player->get_id());
			if (!new_stats.has_value())
			{
				return error(ERR_DATABASE);
			}

			result["sneak_point"] = new_stats->get_fob_point();
			result["is_security_challenge"] = 0;
			result["is_wormhole_open"] = 0;
			result["event_point"] = 0;
		}
		catch (const std::exception&)
		{
			return error(ERR_INVALIDARG);
		}

		return result;
	}
}
