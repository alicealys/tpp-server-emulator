#pragma once

#include "../database.hpp"

#include "fobs.hpp"
#include "players.hpp"
#include "player_data.hpp"

#include "utils/tpp.hpp"

namespace database::fob_event
{
	struct fob_event_point_exchange_param
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

	struct fob_event_information
	{
		std::string important;
		std::uint32_t info_id;
		std::string mes_body;
		std::string mes_subject;
	};

	struct fob_event_motherbase
	{
		nlohmann::json motherbase;
		database::player_data::resource_arrays_t resource_arrays{};
		database::player_data::unit_levels_t unit_levels{};
		database::player_data::unit_counts_t unit_counts{};
		database::player_data::staff_array_t* staff_array{};
		std::uint32_t staff_count;
	};

	struct fob_event_date_range
	{
		std::chrono::seconds start;
		std::chrono::seconds end;
	};

	struct fob_event_player
	{
		std::string player_name;
		nlohmann::json emblem;
		std::vector<std::uint64_t> fob_ids;
		std::vector<database::fobs::fob> fobs;
		fob_event_motherbase motherbase;
		std::uint64_t player_id;
	};

	struct fob_event_t
	{
		std::uint32_t event_id;
		std::string server_text;
		fob_event_date_range date_range;
		std::vector<fob_event_information> information;
		std::vector<fob_event_point_exchange_param> point_exchange_params;
		std::vector<fob_event_player> players;
	};

	std::optional<fob_event_t> get_current_event();
}
