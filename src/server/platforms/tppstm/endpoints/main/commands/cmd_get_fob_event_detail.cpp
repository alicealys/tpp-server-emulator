#include <std_include.hpp>

#include "cmd_get_fob_event_detail.hpp"

#include "database/models/sneak_results.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_event_detail::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& event_index_j = data["event_index"];
		if (!event_index_j.is_number_unsigned())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto event_id = event_index_j.get<std::uint64_t>();
		auto sneak_result = database::sneak_results::get_sneak_result(player->get_id(), event_index_j);

		if (!sneak_result.has_value())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		auto& sneak_data = sneak_result->get_data();

		const auto attacker_record = database::player_records::find(sneak_result->get_player_id());
		if (!attacker_record.has_value())
		{
			result["result"] = "ERR_DATABASE";
			return result;
		}

		result["event_index"] = event_id;
		result["event"] = sneak_data["event"];
		result["event"]["attacker_league_grade"] = attacker_record->get_league_grade();
		result["event"]["attacker_sneak_grade"] = attacker_record->get_fob_grade();
		result["event"]["regist_date"] = sneak_result->get_date();

		return result;
	}
}
