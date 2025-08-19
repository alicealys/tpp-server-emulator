#include <std_include.hpp>

#include "database/models/fob_event.hpp"

#include "target_list_event.hpp"

namespace emulator::tpp
{
	target_list_t target_list_event::generate(const database::players::player& player, const database::player_data::player_data_ptr& player_data, const std::uint32_t limit)
	{
		target_list_t targets;

		const auto fob_event = database::fob_event::get_current_event();
		if (!fob_event.has_value())
		{
			return {};
		}

		auto end = fob_event->date_range.end.count();

		for (const auto& fob_event_player : fob_event->players)
		{
			target_data_t data{};
			data.player_id = fob_event_player.player_id;
			data.extra_data["owner_fob_record"]["date_time"] = end;
			data.extra_data["owner_info"]["player_id"] = 1;
			data.extra_data["owner_info"]["player_name"] = fob_event_player.player_name;
			targets.emplace_back(data);
		}

		return targets;
	}
}
