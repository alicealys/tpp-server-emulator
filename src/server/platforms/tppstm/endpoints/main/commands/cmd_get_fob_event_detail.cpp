#include <std_include.hpp>

#include "cmd_get_fob_event_detail.hpp"

#include "database/models/sneak_results.hpp"
#include "database/models/player_records.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_event_detail::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto& event_index_j = data["event_index"];
		if (!event_index_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto event_id = event_index_j.get<std::uint64_t>();
		auto sneak_result = database::sneak_results::get_sneak_result(player->get_id(), event_id);

		if (!sneak_result.has_value())
		{
			return error(ERR_DATABASE);
		}
		const auto attacker = database::players::find(sneak_result->get_attacker_id());
		if (!attacker.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto attacker_record = database::player_records::find(sneak_result->get_attacker_id());
		if (!attacker_record.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto& event_data = sneak_result->get_event_data();
		const auto& event_log = sneak_result->get_event_log();

		result["event_index"] = event_id;

		result["event"]["cluster"] = event_data.cluster;
		result["event"]["data"] = database::sneak_results::sneak_result::encode_client_event_log(event_log);
		result["event"]["size"] = event_log.size();
		result["event"]["gmp"] = event_data.gmp;
		result["event"]["is_win"] = sneak_result->is_win() ? 1 : 0;
		result["event"]["layout_code"] = event_data.layout_code;
		result["event"]["position_x"] = event_data.position_x;
		result["event"]["position_z"] = event_data.position_z;
		result["event"]["rotate_y"] = event_data.rotate_y;
		result["event"]["capture_nuclear"] = event_data.capture_nuclear;
		result["event"]["capture_resource"]["biotic_resource"] = event_data.capture_resource.biotic_resource;
		result["event"]["capture_resource"]["common_metal"] = event_data.capture_resource.common_metal;
		result["event"]["capture_resource"]["fuel_resource"] = event_data.capture_resource.fuel_resource;
		result["event"]["capture_resource"]["minor_metal"] = event_data.capture_resource.minor_metal;
		result["event"]["capture_resource"]["precious_metal"] = event_data.capture_resource.precious_metal;
		result["event"]["attacker_info"] = player_info(attacker);
		result["event"]["attacker_league_grade"] = attacker_record->get_league_grade();
		result["event"]["attacker_sneak_grade"] = attacker_record->get_fob_grade();
		result["event"]["regist_date"] = sneak_result->get_date();

		return result;
	}
}
