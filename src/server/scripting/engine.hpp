#pragma once

#include "database/models/event_rankings.hpp"
#include "database/models/fobs.hpp"
#include "database/models/items.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_follows.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"
#include "database/models/sneak_results.hpp"
#include "database/models/wormholes.hpp"

#pragma warning(push)
#pragma warning(disable: 4702)
#pragma warning(disable: 5321)

#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 0
#include <sol/sol.hpp>

#include "event_handler.hpp"
#include "scheduler.hpp"

namespace tpp::scripting
{
	class engine
	{
	public:
		engine() = default;

		engine(const engine&) = delete;
		engine& operator=(const engine&) = delete;

		void setup_event_handler();
		void setup_scheduler();

		void setup_server();
		void setup_json();
		void setup_database();
		void setup_player();
		void setup_player_data();
		void setup_player_follow();
		void setup_player_record();
		void setup_sneak_result();
		void setup_event_ranking();
		void setup_wormhole();
		void setup_fob();
		void setup_item();

		void initialize();
		void load_scripts();

		void run_frame();
		void dispatch_event(const std::string& name, const std::vector<std::string>& args);

		void reset();

		std::optional<nlohmann::json> handle_command(const std::string& command, nlohmann::json& data, 
			const std::optional<database::players::player>& player);
		void set_original_handler(const std::function<nlohmann::json()>& original_handler);
		void reset_original_handler();

	private:
		bool initialized_{};

		std::optional<std::function<nlohmann::json()>> original_handler_;

		sol::state state_{};
		std::unordered_map<std::string, sol::protected_function> command_handlers_;

		event_handler event_handler_{};
		scheduler scheduler_{};

	};

	void start();
	void run_frame();
	void dispatch_event(const std::string&, const std::vector<std::string>& args);
	void stop();
	void reload();

	std::optional<nlohmann::json> execute_command_hook(const std::string& command, nlohmann::json& data, 
		const std::optional<database::players::player>& player, const std::function<nlohmann::json()>& original_handler);
}
