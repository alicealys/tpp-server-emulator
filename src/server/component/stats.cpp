#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "scheduler.hpp"
#include "../server.hpp"

#include "database/database.hpp"
#include "database/models/players.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>

namespace stats
{
	namespace
	{
		void update_window_title()
		{
			const auto total_count = database::players::get_player_count();
			const auto active_count = database::players::get_online_player_count();
			const auto title = std::format("tpp-server-emulator - ({} / {} active players)", active_count, total_count);
			SetConsoleTitle(title.data());
		}
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			update_window_title();
			scheduler::loop(update_window_title, scheduler::async, 60s);

			command::add("player_count", [](const command::params& params)
			{
				std::chrono::seconds within = database::players::session_timeout;

				if (params.size() > 1)
				{
					const auto time_str = params.get(1);
					within = std::chrono::seconds{std::strtoull(time_str.data(), nullptr, 10)};
				}

				const auto total_count = database::players::get_player_count();
				const auto count = database::players::get_online_player_count(within);
				console::log("Active players (within %llis): %lli / %lli\n", within.count(), count, total_count);
			});
		}
	};
}

REGISTER_COMPONENT(stats::component)
