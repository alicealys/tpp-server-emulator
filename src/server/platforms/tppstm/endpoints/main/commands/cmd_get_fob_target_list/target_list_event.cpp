#include <std_include.hpp>

#include "database/models/fob_events.hpp"

#include "target_list_event.hpp"

namespace emulator::tpp
{
	target_list_t target_list_event::generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto fob_event = database::fob_events::get_current_event();
		if (!fob_event.has_value())
		{
			return {};
		}

		auto end = fob_event->date_range.end.count();

		for (const auto& player_id : fob_event->player_ids)
		{
			const auto event_player = database::fob_events::get_player(player_id);
			if (!event_player.has_value())
			{
				continue;
			}

			for (const auto& fob_id : event_player->fob_ids)
			{
				target_data_t data{};

				data.player_id = event_player->player_id;

				data.extra_data["owner_fob_record"]["date_time"] = end;
				data.extra_data["owner_info"]["player_name"] = event_player->player_name;
				data.extra_data["owner_detail_record"]["staff_count"] = 0;

				data.fob_filter.emplace([fob_id](const database::fobs::fob& other_fob)
				{
					return other_fob.get_id() == fob_id;
				});

				targets.emplace_back(data);
			}
		}

		return targets;
	}
}
