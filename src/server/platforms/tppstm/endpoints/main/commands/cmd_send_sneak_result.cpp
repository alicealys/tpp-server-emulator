#include <std_include.hpp>

#include "cmd_send_sneak_result.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_send_sneak_result::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		const auto stats = database::player_records::find(player->get_id());
		if (!stats.has_value())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

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
				result["result"] = "ERR_INVALIDARG";
				return result;
			}

			const auto mode = database::players::get_sneak_mode_id(mode_str);
			if (mode == database::players::mode_invalid)
			{
				result["result"] = "ERR_INVALIDARG";
				return result;
			}

			if (mode == database::players::mode_actual)
			{
				nlohmann::json sneak_data = data;
				sneak_data.erase("msgid");
				sneak_data.erase("rqid");

				database::player_records::add_sneak_result(player->get_id(), sneak_point, is_win);
				database::sneak_results::add_sneak_result(player->get_id(), fob->get_player_id(), fob->get_id(), sneak_data);
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
