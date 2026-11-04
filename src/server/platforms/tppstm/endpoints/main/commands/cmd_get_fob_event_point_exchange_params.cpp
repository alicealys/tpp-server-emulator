#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/items.hpp"

#include "cmd_get_fob_event_point_exchange_params.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_event_point_exchange_params::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
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

		const auto items = database::items::get_item_list(player->get_id(), false);
		const auto is_open = [&](const std::uint32_t id)
		{
			const auto iter = items.find(id);
			return iter != items.end() && iter->second.is_open();
		};

		auto count = 0u;
		for (auto i = 0ull; i < fob_event->point_exchange_params.size(); i++)
		{
			auto limit = fob_event->point_exchange_params[i].exchange_limit;
			if ((fob_event->point_exchange_params[i].type == 3 || fob_event->point_exchange_params[i].type == 4) && 
				is_open(fob_event->point_exchange_params[i].unique_id))
			{
				limit = 0;
			}

			auto& entry = result["param_list"][count++];
			entry["common_value"] = fob_event->point_exchange_params[i].common_value;
			entry["count"] = fob_event->point_exchange_params[i].count;
			entry["exchange_limit"] = limit;
			entry["info_lang_id"] = fob_event->point_exchange_params[i].info_lang_id;
			entry["limited_count"] = fob_event->point_exchange_params[i].limited_count;
			entry["name_lang_id"] = fob_event->point_exchange_params[i].name_lang_id;
			entry["point"] = fob_event->point_exchange_params[i].point;
			entry["type"] = fob_event->point_exchange_params[i].type;
			entry["unique_id"] = fob_event->point_exchange_params[i].unique_id;
		}

		result["param_num"] = count;

		return result;
	}
}
