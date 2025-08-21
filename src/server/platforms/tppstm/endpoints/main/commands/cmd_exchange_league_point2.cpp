#include <std_include.hpp>

#include "database/models/fob_events.hpp"
#include "database/models/player_records.hpp"

#include "cmd_exchange_league_point2.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_exchange_league_point2::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto current_event = database::fob_events::get_current_event();

		const auto& is_event_j = data["is_event"];
		const auto& num_to_exchange_j = data["num_to_exchange"];
		const auto& unique_id_j = data["unique_id"];
		const auto& type_j = data["type"];

		if (!is_event_j.is_number_unsigned() || !num_to_exchange_j.is_number_unsigned() || !unique_id_j.is_number_unsigned() ||
			!type_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_event = is_event_j.get<std::uint32_t>() == 1;
		if (is_event && !current_event.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		result["is_event"] = is_event;

		const auto do_event_exchange = [&]
		{
			const auto num_to_exchange = num_to_exchange_j.get<std::uint32_t>();
			const auto unique_id = unique_id_j.get<std::uint32_t>();
			const auto type = type_j.get<std::uint32_t>();

			const auto iter = std::find_if(current_event->point_exchange_params.begin(), current_event->point_exchange_params.end(),
				[&](const database::fob_events::fob_event_point_exchange_param_t& param)
				{
					return param.unique_id == unique_id && param.type == type;
				}
			);

			if (iter == current_event->point_exchange_params.end())
			{
				return ERR_INVALIDARG;
			}

			const auto point = iter->point * num_to_exchange;
			if (!database::player_records::spend_event_points(player->get_id(), point))
			{
				return ERR_POINT_SHORTAGE;
			}

			const auto stats = database::player_records::find(player->get_id());

			result["point"] = stats->get_event_point();
			return NOERR;
		};

		if (is_event)
		{
			const auto res = do_event_exchange();
			if (res != NOERR)
			{
				return error(res);
			}
		}
		else
		{
			return error(ERR_INVALIDARG); // not implemented
		}

		return result;
	}

	bool cmd_exchange_league_point2::needs_player()
	{
		return true;
	}
}
