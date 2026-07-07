#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/pf_league.hpp"

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

		const auto serialize_list = [&](const database::fob_events::point_exchange_params_t& params)
		{
			for (auto i = 0ull; i < params.size(); i++)
			{
				result["param_list"][i]["common_value"] = params[i].common_value;
				result["param_list"][i]["count"] = params[i].count;
				result["param_list"][i]["exchange_limit"] = params[i].exchange_limit;
				result["param_list"][i]["info_lang_id"] = params[i].info_lang_id;
				result["param_list"][i]["limited_count"] = params[i].limited_count;
				result["param_list"][i]["name_lang_id"] = params[i].name_lang_id;
				result["param_list"][i]["point"] = params[i].point;
				result["param_list"][i]["type"] = params[i].type;
				result["param_list"][i]["unique_id"] = params[i].unique_id;
			}

			result["param_num"] = params.size();
		};

		const auto is_event = is_event_j.get<std::uint32_t>() == 1;
		const auto fob_event = database::fob_events::get_current_event();
		if (is_event && fob_event.has_value())
		{
			serialize_list(fob_event->point_exchange_params);
		}
		else
		{
			const auto& pf_params = database::pf_league::get_point_exchange_params();
			serialize_list(pf_params);
		}

		return result;
	}
}
