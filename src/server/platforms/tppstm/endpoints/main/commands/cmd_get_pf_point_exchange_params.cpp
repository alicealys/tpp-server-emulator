#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/pf_league.hpp"
#include "database/models/items.hpp"
#include "database/models/event_rankings.hpp"

#include "cmd_get_pf_point_exchange_params.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_pf_point_exchange_params::generate(const std::optional<database::players::player>& player, const bool is_event)
	{
		nlohmann::json result;

		result["param_list"] = nlohmann::json::array();
		result["param_num"] = 0;

		const auto items = database::items::get_item_list(player->get_id(), false);
		const auto nameplate_entitlements = database::event_rankings::get_nameplate_entitlement_list(player->get_id());

		const auto is_open = [&](const std::uint32_t id)
		{
			const auto iter = items.find(id);
			return iter != items.end() && iter->is_open();
		};

		const auto serialize_list = [&](const database::fob_events::point_exchange_params_t& params)
		{
			auto count = 0u;
			for (auto i = 0ull; i < params.size(); i++)
			{
				auto& param = params[i];
				auto limit = param.exchange_limit;

				switch (param.type)
				{
				case 3:
				case 4:
				{
					if (!is_open(param.unique_id))
					{
						limit = 0u;
					}

					break;
				}
				case 8:
				{
					if (!nameplate_entitlements.has_nameplate(param.unique_id))
					{
						continue;
					}

					break;
				}
				}

				auto& entry = result["param_list"][count++];
				entry["common_value"] = param.common_value;
				entry["count"] = param.count;
				entry["exchange_limit"] = limit;
				entry["info_lang_id"] = param.info_lang_id;
				entry["limited_count"] = param.limited_count;
				entry["name_lang_id"] = param.name_lang_id;
				entry["point"] = param.point;
				entry["type"] = param.type;
				entry["unique_id"] = param.unique_id;
			}

			result["param_num"] = count;
		};

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

	nlohmann::json cmd_get_pf_point_exchange_params::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto& is_event_j = data["is_event"];
		if (!is_event_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_event = is_event_j.get<std::uint32_t>() != 0;
		return this->generate(player, is_event);
	}
}
