#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/player_records.hpp"
#include "database/models/pf_league.hpp"

#include "cmd_exchange_league_point2.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_exchange_league_point2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& common_value_j = data["common_value"];
		const auto& is_event_j = data["is_event"];
		const auto& num_to_exchange_j = data["num_to_exchange"];
		const auto& unique_id_j = data["unique_id"];
		const auto& type_j = data["type"];

		if (!is_event_j.is_number_unsigned() || !num_to_exchange_j.is_number_unsigned() || !unique_id_j.is_number_unsigned() ||
			!type_j.is_number_unsigned() || !common_value_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_event = is_event_j.get<std::uint32_t>() == 1;

		const auto current_event = database::fob_events::get_current_event();
		const auto& league_params = database::pf_league::get_point_exchange_params();

		if (is_event && !current_event.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& params = is_event
			? current_event->point_exchange_params
			: league_params;

		result["is_event"] = is_event;

		const auto common_value = common_value_j.get<std::uint32_t>();
		const auto num_to_exchange = num_to_exchange_j.get<std::uint32_t>();
		const auto unique_id = unique_id_j.get<std::uint32_t>();
		const auto type = type_j.get<std::uint32_t>();

		const auto iter = std::find_if(params.begin(), params.end(),
			[&](const database::fob_events::point_exchange_param_t& param)
			{
				return param.unique_id == unique_id && param.type == type && param.common_value == common_value;
			}
		);

		if (iter == params.end())
		{
			return error(ERR_INVALIDARG);
		}

		const auto point = iter->point * num_to_exchange;
		const auto spend_fn = is_event
			? database::player_records::spend_event_points
			: database::player_records::spend_pf_points;

		if (!spend_fn(player->get_id(), point))
		{
			return error(ERR_INVALIDARG);
		}

		if (iter->type == 2)
		{
			database::player_data::give_resource(player->get_id(), iter->unique_id, iter->count);
		}

		const auto stats = database::player_records::find(player->get_id());

		result["point"] = is_event 
			? stats->get_event_point()
			: stats->get_pf_point();

		return result;
	}

	bool cmd_exchange_league_point2::needs_player()
	{
		return true;
	}
}
