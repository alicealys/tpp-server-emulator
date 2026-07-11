#include <std_include.hpp>

#include "cmd_check_short_pfleague_enterable.hpp"
#include "database/models/pf_league.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_check_short_pfleague_enterable::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["result_already_read"] = 0;
		result["status"] = "ACCEPTING";
		result["pfleague_date"] = 0;

		const auto current_application = database::pf_league::get_current_league_application(player->get_id());
		if (!current_application.has_value())
		{
			return result;
		}

		if (current_application->get_league_id() == 0)
		{
			result["status"] = "ALREADY_ENTERED";

			std::chrono::system_clock::time_point start{};
			std::chrono::system_clock::time_point end{};
			database::pf_league::get_short_pf_league_range(start, end);

			std::chrono::hours start_offset{};
			std::chrono::hours duration{};
			database::pf_league::get_short_pf_league_duration(start_offset, duration);

			const auto date = std::chrono::duration_cast<std::chrono::seconds>((end + start_offset).time_since_epoch());
			result["pfleague_date"] = date.count();
		}
		else
		{
			const auto league = database::pf_league::get_league(current_application->get_league_id());
			result["status"] = "ALREADY_HELD";
			result["pfleague_date"] = league->get_start_date().count();
		}

		result["result_already_read"] = 1;

		const auto previous_application = database::pf_league::get_previous_league_application(player->get_id());
		if (previous_application.has_value() && previous_application->get_read_state() == 0)
		{
			const auto previous_league = database::pf_league::get_league(previous_application->get_league_id());
			const auto now = std::chrono::system_clock::now();
			const auto now_s = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

			if (previous_league.has_value() && previous_league->get_end_date() < now_s)
			{
				result["result_already_read"] = 0;
			}

			database::pf_league::set_league_application_read_state(current_application->get_id(), 1);
		}

		return result;
	}

	bool cmd_check_short_pfleague_enterable::needs_player()
	{
		return true;
	}
}
