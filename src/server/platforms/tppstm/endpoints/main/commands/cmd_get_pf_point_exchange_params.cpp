#include <std_include.hpp>

#include "database/models/fob_events.hpp"

#include "cmd_get_pf_point_exchange_params.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_pf_point_exchange_params::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& is_event_j = data["is_event"];
		if (!is_event_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		result["param_list"] = nlohmann::json::array();
		result["param_num"] = 0;

		const auto is_event = is_event_j.get<std::uint32_t>() == 1;
		if (!is_event)
		{
			return result;
		}

		const auto fob_event = database::fob_events::get_current_event();
		if (!fob_event.has_value())
		{
			return {};
		}

		for (auto i = 0ull; i < fob_event->point_exchange_params.size(); i++)
		{
			result["param_list"][i]["common_value"] = fob_event->point_exchange_params[i].common_value;
			result["param_list"][i]["count"] = fob_event->point_exchange_params[i].count;
			result["param_list"][i]["exchange_limit"] = fob_event->point_exchange_params[i].exchange_limit;
			result["param_list"][i]["info_lang_id"] = fob_event->point_exchange_params[i].info_lang_id;
			result["param_list"][i]["limited_count"] = fob_event->point_exchange_params[i].limited_count;
			result["param_list"][i]["name_lang_id"] = fob_event->point_exchange_params[i].name_lang_id;
			result["param_list"][i]["point"] = fob_event->point_exchange_params[i].point;
			result["param_list"][i]["type"] = fob_event->point_exchange_params[i].type;
			result["param_list"][i]["unique_id"] = fob_event->point_exchange_params[i].unique_id;
		}

		result["param_num"] = fob_event->point_exchange_params.size();

		return result;
	}
}
