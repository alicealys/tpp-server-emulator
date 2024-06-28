#include <std_include.hpp>

#include "cmd_get_fob_event_list.hpp"

#include "database/models/sneak_results.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_fob_event_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		auto sneak_results = database::sneak_results::get_sneak_results(player->get_id(), 10);

		for (auto i = 0u; i < sneak_results.size(); i++)
		{
			auto& sneak = sneak_results[i];
			auto& sneak_data = sneak.get_data();

			const auto layout_code = sneak_data["event"]["layout_code"].get<std::uint32_t>();

			result["event_list"][i]["attacker_id"] = sneak.get_player_id();
			result["event_list"][i]["event_index"] = sneak.get_id();
			result["event_list"][i]["fob_index"] = sneak.get_fob_index();
			result["event_list"][i]["is_win"] = sneak.is_win();
			result["event_list"][i]["cluster"] = sneak.get_platform();
			result["event_list"][i]["layout_code"] = layout_code;
		}

		result["event_num"] = sneak_results.size();

		return result;
	}
}
