#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/pf_league.hpp"
#include "database/models/items.hpp"

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

		const auto items = database::items::get_item_list(player->get_id(), false);
		const auto is_open = [&](const std::uint32_t id)
		{
			const auto iter = items.find(id);
			return iter != items.end() && iter->second.is_open();
		};
		
		const auto serialize_list = [&](const database::fob_events::point_exchange_params_t& params)
		{
			auto count = 0u;
			for (auto i = 0ull; i < params.size(); i++)
			{
				auto limit = params[i].exchange_limit;
				if ((params[i].type == 3 || params[i].type == 4) && 
					is_open(params[i].unique_id))
				{
					limit = 0;
				}

				auto& entry = result["param_list"][count++];
				entry["common_value"] = params[i].common_value;
				entry["count"] = params[i].count;
				entry["exchange_limit"] = limit;
				entry["info_lang_id"] = params[i].info_lang_id;
				entry["limited_count"] = params[i].limited_count;
				entry["name_lang_id"] = params[i].name_lang_id;
				entry["point"] = params[i].point;
				entry["type"] = params[i].type;
				entry["unique_id"] = params[i].unique_id;
			}

			result["param_num"] = count;
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
