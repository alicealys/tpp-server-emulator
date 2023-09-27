#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_send_sneak_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.get())
		{
			result["result"] = "ERR_DATABASE";
			return result;
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
				result["result"] = "ERR_DATABASE";
				return result;
			}

			const auto mode = database::players::get_sneak_mode_id(mode_str);
			if (mode == database::players::mode_invalid)
			{
				result["result"] = "ERR_INVALIDARG";
				return result;
			}

			const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());
			if (!active_sneak.has_value())
			{
				result["result"] = "ERR_DATABSE";
				return result;
			}

			if (active_sneak->get_mode() != mode)
			{
				result["result"] = "ERR_DATABSE";
				return result;
			}

			if (active_sneak->get_mode() == database::players::mode_actual)
			{
				nlohmann::json sneak_data = data;
				sneak_data.erase("msgid");
				sneak_data.erase("rqid");

				sneak_data["event"]["attacker_info"] = {};
				sneak_data["event"]["attacker_info"]["npid"]["handler"]["data"] = "";
				sneak_data["event"]["attacker_info"]["npid"]["handler"]["dummy"] = {0, 0, 0};
				sneak_data["event"]["attacker_info"]["npid"]["handler"]["term"] = 0;
				sneak_data["event"]["attacker_info"]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
				sneak_data["event"]["attacker_info"]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
				sneak_data["event"]["attacker_info"]["player_id"] = player->get_id();
				sneak_data["event"]["attacker_info"]["player_name"] = player->get_name();
				sneak_data["event"]["attacker_info"]["ugc"] = 1;
				sneak_data["event"]["attacker_info"]["xuid"] = player->get_account_id();

				const auto is_sneak = active_sneak->is_sneak();
				database::player_records::add_sneak_result(player->get_id(), fob->get_player_id(), sneak_point, is_win, is_sneak);

				if (is_sneak)
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

					auto& active_sneak_val = active_sneak.value();
					if (!database::sneak_results::add_sneak_result(player.value(), fob.value(), active_sneak_val, is_win, sneak_data))
					{
						result["result"] = "ERR_DATABASE";
					}
				}
			}

			const auto new_stats = database::player_records::find(player->get_id());
			if (!new_stats.has_value())
			{
				result["result"] = "ERR_DATABASE";
				return result;
			}

			result["sneak_point"] = new_stats->get_fob_point();
			result["is_security_challenge"] = 0;
			result["is_wormhole_open"] = 0;
			result["event_point"] = 0;
		}
		catch (const std::exception&)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		return result;
	}
}
