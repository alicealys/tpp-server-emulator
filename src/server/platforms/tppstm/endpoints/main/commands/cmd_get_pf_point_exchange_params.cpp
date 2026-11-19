#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/pf_league.hpp"
#include "database/models/items.hpp"
#include "database/models/event_rankings.hpp"
#include "database/models/point_exchanges.hpp"

#include "cmd_get_pf_point_exchange_params.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_pf_point_exchange_params::generate(const std::optional<database::players::player>& player, const bool is_event)
	{
		nlohmann::json result;

		auto& param_list_j = result["param_list"];

		param_list_j = nlohmann::json::array();
		result["param_num"] = 0;

		const auto items = database::items::get_item_list(player->get_id(), false);
		const auto nameplate_entitlements = database::event_rankings::get_nameplate_entitlement_list(player->get_id());

		const auto is_open = [&](const std::uint32_t id)
		{
			const auto iter = items.find(id);
			return iter != items.end() && iter->is_open();
		};

		const auto serialize_list = [&](const database::point_exchanges::point_exchange_params_t& params, const database::point_exchanges::point_exchange_params_t& exchanged_params)
		{
			auto count = 0u;
			for (auto i = 0ull; i < params.size(); i++)
			{
				database::point_exchanges::point_exchange_param_t param = params[i];
				auto limit = param.exchange_limit;

				switch (param.type)
				{
				case database::point_exchanges::exchange_type_blueprint:
				case database::point_exchanges::exchange_type_uniform:
				{
					if (!is_open(param.unique_id))
					{
						limit = 0u;
					}

					break;
				}
				case database::point_exchanges::exchange_type_nameplate:
				{
					if (!nameplate_entitlements.has_nameplate(param.unique_id))
					{
						continue;
					}

					break;
				}
				}

				param.exchange_limit = database::point_exchanges::get_exchange_limit(param, exchanged_params);
				if (param.exchange_limit == 0)
				{
					continue;
				}

				param.to_json(param_list_j[count++]);
			}

			result["param_num"] = count;
		};

		const auto current_event = database::fob_events::get_current_event();
		const auto current_league = database::pf_league::get_current_pf_league();
		const auto exchanged_params = database::point_exchanges::get_exchanged_params(player->get_id(), is_event 
			? database::point_exchanges::exchange_mode_event 
			: database::point_exchanges::exchange_mode_league);

		if (is_event && current_event != nullptr)
		{
			serialize_list(current_event->point_exchange_params, exchanged_params);
		}
		else if (!is_event && current_league.has_value())
		{
			const auto& pf_params = database::pf_league::get_point_exchange_params(current_league->get_id());
			serialize_list(pf_params, exchanged_params);
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
