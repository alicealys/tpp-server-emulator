#pragma once

#include "../../platforms/tppstm/endpoints/main/commands/cmd_get_informationlist2.hpp"
#include "../database.hpp"

#include "fobs.hpp"
#include "players.hpp"
#include "player_data.hpp"
#include "point_exchanges.hpp"

namespace database::fob_events
{
	constexpr const auto event_intermission_duration = 8h;

	struct fob_event_information_t
	{
		std::string important;
		std::uint32_t info_id;
		std::string mes_body;
		std::string mes_subject;
	};

	struct fob_even_staff_resources_t
	{
		database::player_data::resource_arrays_t resource_arrays{};
		database::player_data::unit_levels_t unit_levels{};
		database::player_data::unit_counts_t unit_counts{};
		database::player_data::staff_array_container staff_array;
		std::uint32_t staff_count;
	};

	struct fob_event_date_range_t
	{
		std::chrono::seconds start;
		std::chrono::seconds end;
	};

	struct fob_event_player_t
	{
		std::uint64_t player_id;
		std::string player_name;
		game::emblem_t emblem;
		game::motherbase_t motherbase;
		std::vector<std::uint64_t> fob_ids;
		std::vector<game::fob_param_t> fob_params;
		fob_even_staff_resources_t staff_resources;

		bool parse(nlohmann::json& data);
	};

	struct fob_event_t
	{
		std::uint32_t index;
		std::uint32_t number;
		std::string server_text;
		nlohmann::json one_event_task;
		fob_event_date_range_t date_range;
		std::vector<emulator::tpp::cmd_get_informationlist2_base::message_t> information;
		point_exchanges::point_exchange_params_t point_exchange_params;
		std::vector<std::uint64_t> player_ids;
		std::vector<std::shared_ptr<fob_event_player_t>> players;

		bool parse(nlohmann::json& data);
	};

	struct fob_events_data_t
	{
		std::vector<std::shared_ptr<fob_event_player_t>> players;
		std::vector<std::shared_ptr<fob_event_t>> events;

		bool parse(nlohmann::json& data);
		std::shared_ptr<fob_event_player_t> get_player(const std::uint64_t id);
	};

	struct running_event_info_t
	{
		std::chrono::seconds start;
		std::chrono::seconds end;
		std::uint32_t index;
		std::uint32_t number;
		std::shared_ptr<fob_event_t> params;
	};

	void get_maintenance_range(std::chrono::system_clock::time_point& start, std::chrono::system_clock::time_point& end);
	void get_event_range(std::chrono::seconds& start, std::chrono::seconds& end);

	
	std::optional<running_event_info_t> get_current_event();

	bool is_intermission();
	bool is_event_text(const std::string& text);
}
