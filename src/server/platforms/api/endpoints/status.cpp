#include <std_include.hpp>

#include "status.hpp"
#include "database/auth.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/fob_events.hpp"
#include "server.hpp"

#include <utils/http.hpp>

namespace emulator
{
	nlohmann::json status::handle_request(const utils::request_params& params, const std::string&)
	{
		nlohmann::json result;

		const auto now = std::chrono::system_clock::now();
		const auto uptime = now - get_server_startup();
		const auto uptime_s = std::chrono::duration_cast<std::chrono::seconds>(uptime).count();

		result["uptime"] = uptime_s;
		result["online_player_count"] = database::players::get_online_player_count();
		result["total_player_count"] = database::players::get_player_count();
		result["nuke_count"] = database::player_data::get_nuke_count();

		const auto current_event = database::fob_events::get_current_event();
		if (current_event.has_value())
		{
			result["event"]["start_date"] = current_event->date_range.start.count();
			result["event"]["end_date"] = current_event->date_range.end.count();
			result["event"]["server_text"] = current_event->server_text;
		}

		return result;
	}
}
