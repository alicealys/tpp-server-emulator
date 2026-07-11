#include <std_include.hpp>

#include "cmd_enter_short_pfleague.hpp"

#include "database/models/pf_league.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_enter_short_pfleague::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto current_application = database::pf_league::get_current_league_application(player->get_id());
		if (current_application.has_value() && current_application->get_league_id() == 0)
		{
			return error(ERR_DATABASE);
		}

		if (database::pf_league::create_league_application(player->get_id()))
		{
			std::chrono::system_clock::time_point start{};
			std::chrono::system_clock::time_point end{};
			database::pf_league::get_short_pf_league_range(start, end);

			std::chrono::hours start_offset{};
			std::chrono::hours duration{};
			database::pf_league::get_short_pf_league_duration(start_offset, duration);

			const auto date = std::chrono::duration_cast<std::chrono::seconds>((end + start_offset).time_since_epoch());

			result["pfleague_date"] = date.count();
			result["status"] = "SUCCESS";
		}
		else
		{
			result["pfleague_date"] = 0;
			result["status"] = "FAIL";
		}

		return result;
	}

	bool cmd_enter_short_pfleague::needs_player()
	{
		return true;
	}
}
