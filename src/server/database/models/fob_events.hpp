#pragma once

#include "../../platforms/tppstm/endpoints/main/commands/cmd_get_informationlist2.hpp"
#include "../database.hpp"

#include "fobs.hpp"
#include "players.hpp"
#include "player_data.hpp"
#include "point_exchanges.hpp"
#include "online_news.hpp"

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
		std::string name;
		std::uint32_t index;
		std::uint32_t number;
		std::string server_text;
		nlohmann::json one_event_task;
		fob_event_date_range_t date_range;
		std::vector<online_news::information_data_t> information;
		point_exchanges::point_exchange_params_t point_exchange_params;
		std::vector<std::uint64_t> player_ids;
		std::vector<std::shared_ptr<fob_event_player_t>> players;

		bool parse(nlohmann::json& data);
	};

	struct fob_events_settings_t
	{
		std::uint32_t event_duration;
		std::vector<std::shared_ptr<fob_event_player_t>> event_players;
		std::vector<std::shared_ptr<fob_event_t>> event_list;
		std::vector<std::shared_ptr<fob_event_t>> event_rotation;

		bool parse(nlohmann::json& data);
		std::shared_ptr<fob_event_player_t> get_player(const std::uint64_t id);
	};

	struct running_event_info_t
	{
		std::chrono::seconds start;
		std::chrono::seconds end;
		std::size_t index;
		std::size_t number;
		std::shared_ptr<fob_event_t> params;
	};

	bool is_intermission();
	bool is_event_text(const std::string& text);

	enum event_state_t
	{
		event_state_none = 0u,
		event_state_running = 1u,
		event_state_finished = 2u,
		event_state_dead = 3u,
	};

	enum event_ranking_state_t
	{
		ranking_state_none = 0u,
		ranking_state_ack = 1u
	};

	class fob_event
	{
	public:
		DEFINE_FIELD(event_id, sqlpp::integer_unsigned);
		DEFINE_FIELD(event_type, sqlpp::text);
		DEFINE_FIELD(event_state, sqlpp::integer_unsigned);
		DEFINE_FIELD(start_date, sqlpp::time_point);
		DEFINE_FIELD(end_date, sqlpp::time_point);
		DEFINE_TABLE(fob_events,
			event_id_field_t,
			event_type_field_t,
			event_state_field_t,
			start_date_field_t,
			end_date_field_t
		);

		inline static table_t table;

		template <typename ...Args>
		fob_event(const sqlpp::result_row_t<Args...>& row)
		{
			this->event_id_ = row.event_id;
			this->event_type_ = row.event_type;
			this->event_state_ = static_cast<std::uint32_t>(row.event_state);
			this->start_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.start_date.value().time_since_epoch());
			this->end_date_ = std::chrono::duration_cast<std::chrono::seconds>(row.end_date.value().time_since_epoch());
		}

		bool initialize_params();

		GET_FIELD_H(std::uint64_t, event_id);
		GET_FIELD_H(std::string, event_type);
		GET_FIELD_H(std::uint32_t, event_state);
		GET_FIELD_H(std::chrono::seconds, start_date);
		GET_FIELD_H(std::chrono::seconds, end_date);

		std::shared_ptr<fob_event_t> params;

	};

	std::optional<fob_event> get_current_event();

}
