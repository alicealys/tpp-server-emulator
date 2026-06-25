#include <std_include.hpp>

#include "cmd_get_fob_event_list.hpp"

#include "database/models/sneak_results.hpp"

namespace emulator::tpp
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
			const auto& event_data = sneak.get_event_data();

			result["event_list"][i]["attacker_id"] = sneak.get_attacker_id();
			result["event_list"][i]["event_index"] = sneak.get_id();
			result["event_list"][i]["fob_index"] = sneak.get_fob_index();
			result["event_list"][i]["is_win"] = sneak.is_win();
			result["event_list"][i]["cluster"] = sneak.get_platform();
			result["event_list"][i]["layout_code"] = event_data.layout_code;
		}

		result["event_num"] = sneak_results.size();

		return result;
	}
}
