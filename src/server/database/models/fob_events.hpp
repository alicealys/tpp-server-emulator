#pragma once

#include "../../platforms/tppstm/endpoints/main/commands/cmd_get_informationlist2.hpp"
#include "../database.hpp"

#include "fobs.hpp"
#include "players.hpp"
#include "player_data.hpp"

namespace database::fob_events
{
	struct fob_event_point_exchange_param_t
	{
		std::uint32_t common_value;
		std::uint32_t count;
		std::uint32_t exchange_limit;
		std::uint32_t info_lang_id;
		std::uint32_t limited_count;
		std::uint32_t name_lang_id;
		std::uint32_t point;
		std::uint32_t type;
		std::uint32_t unique_id;
	};

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
		nlohmann::json reward;
		std::vector<std::uint64_t> fob_ids;
		std::vector<game::fob_param_t> fob_params;
		fob_even_staff_resources_t staff_resources;
	};

	struct fob_event_t
	{
		std::string server_text;
		nlohmann::json one_event_task;
		fob_event_date_range_t date_range;
		std::vector<emulator::tpp::information_list_entry_t> information;
		std::vector<fob_event_point_exchange_param_t> point_exchange_params;
		std::vector<std::uint64_t> player_ids;
	};

	struct fob_events_data_t
	{
		std::vector<fob_event_player_t> players;
		std::vector<fob_event_t> events;
	};

	std::optional<fob_event_player_t> get_player(const std::uint64_t id);
	bool is_event_player(const std::uint64_t id);
	std::optional<fob_event_t> get_current_event();
}
