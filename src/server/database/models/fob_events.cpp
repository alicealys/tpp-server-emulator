#include <std_include.hpp>

#include "fob_events.hpp"
#include "players.hpp"
#include "player_data.hpp"
#include "player_records.hpp"
#include "fobs.hpp"
#include "variables.hpp"
#include "event_rankings.hpp"
#include "mgo_data.hpp"
#include "online_news.hpp"
#include "../../component/command.hpp"

#include "utils/lang_strings.hpp"
#include "utils/time.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::fob_events
{
	namespace
	{
		void register_lang_vars()
		{
			utils::lang::register_var("event_name", [](const std::uint8_t lang)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return utils::lang::get_string(current_event->params->server_text, lang);
			});

			utils::lang::register_var("event_start", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%b. %d, %Y", std::chrono::system_clock::time_point(current_event->get_start_date()));
			});

			utils::lang::register_var("event_end", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%b. %d, %Y", std::chrono::system_clock::time_point(current_event->get_end_date()));
			});

			utils::lang::register_var("event_start_ymd", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%Y/%m/%d", std::chrono::system_clock::time_point(current_event->get_start_date()));
			});

			utils::lang::register_var("event_end_ymd", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%Y/%m/%d", std::chrono::system_clock::time_point(current_event->get_end_date()));
			});

			utils::lang::register_var("event_start_ymd_rev", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%d/%m/%Y", std::chrono::system_clock::time_point(current_event->get_start_date()));
			});

			utils::lang::register_var("event_end_ymd_rev", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%d/%m/%Y", std::chrono::system_clock::time_point(current_event->get_end_date()));
			});
		}

		void create_database_entries_for_player(fob_event_player_t& event_player)
		{
			const auto player = database::players::create_system_player(event_player.player_id);

			const auto player_data = database::player_data::find_or_create(player.get_id());
			database::player_data::sync_emblem(player.get_id(), event_player.emblem);
			database::player_data::sync_motherbase(player.get_id(), event_player.motherbase);
			database::player_data::set_resources_gmp(player.get_id(), event_player.staff_resources.resource_arrays, 0, 0);

			database::player_data::staff_info_t staff_info{};
			database::player_data::set_staff(player.get_id(), player_data->get_server_staff_version(),
				event_player.staff_resources.staff_array, staff_info);

			database::player_data::set_staff_levels(player.get_id(), event_player.staff_resources.unit_levels,
				event_player.staff_resources.unit_counts);


			database::player_records::find_or_create(player.get_id());

			const auto fob_list = database::fobs::get_fob_list(player.get_id());
			if (fob_list.empty())
			{
				for (auto i = 0ull; i < event_player.fob_params.size(); i++)
				{
					database::fobs::create(player.get_id(), 0u, event_player.fob_ids[i]);
				}
			}

			const auto new_fob_list = database::fobs::get_fob_list(player.get_id());
			database::fobs::sync_data(player.get_id(), event_player.fob_params);
		}

		fob_events_settings_t load_event_settings()
		{
			fob_events_settings_t settings{};

			auto data = utils::resources::load_json(RESOURCE_FOB_EVENT_LIST);
			if (!settings.parse(data))
			{
				throw std::runtime_error("[fob events] failed to parse fob event settings");
			}

			for (auto& player : settings.event_players)
			{
				create_database_entries_for_player(*player);
			}

			for (auto& event : settings.event_list)
			{
				for (auto& player_id : event->player_ids)
				{
					const auto player = settings.get_player(player_id);
					if (player == nullptr)
					{
						throw std::runtime_error(std::format("[fob events] invalid player id {}", player_id));
					}

					event->players.emplace_back(player);
				}
			}

			return settings;
		}

		const fob_events_settings_t& get_event_settings()
		{
			static const auto event_list = load_event_settings();
			return event_list;
		}

		std::optional<running_event_info_t> get_target_event()
		{
			const auto& settings = get_event_settings();
			if (settings.event_rotation.empty())
			{
				return {};
			}

			running_event_info_t event{};

			std::size_t section_number{};
			utils::time::get_section(settings.event_duration * 24h, event.start, event.end, section_number);

			event.index = section_number % settings.event_rotation.size();
			event.number = section_number;
			event.params = settings.event_rotation[event.index];

			return {event};
		}

		void reset_values()
		{
			database::event_rankings::reset_periodic_values();
			database::player_records::reset_event_points();
			database::mgo_data::reset_survival_tickets(database::mgo_data::base_survival_tickets);
			players::disable_all_security_challenges();
		}
	}

	bool fob_event_player_t::parse(nlohmann::json& data)
	{
		this->player_id = data["player_id"].get<std::uint64_t>();
		this->player_name = data["player_name"].get<std::string>();

		if (!this->emblem.parse(data["emblem"], false))
		{
			return false;
		}

		if (!this->motherbase.parse(data["motherbase"]))
		{
			return false;
		}

		auto& fob_list_j = data["fob_list"];
		for (auto i = 0ull; i < fob_list_j.size(); i++)
		{
			this->fob_ids.emplace_back(fob_list_j[i]["mother_base_id"]);

			game::fob_param_t param{};
			if (!param.parse(fob_list_j[i]))
			{
				return false;
			}

			this->fob_params.emplace_back(param);
		}

		auto& staff_resources_j = data["staff_resources"];
		if (!staff_resources_j.is_object())
		{
			return false;
		}

		game::fob_placements_t placement{};
		game::fob_resources_t usable_resource{};
		game::fob_resources_t processing_resource{};

		if (!placement.parse(staff_resources_j["placement"]) ||
			!usable_resource.parse(staff_resources_j["usable_resource"]) ||
			!processing_resource.parse(staff_resources_j["processing_resource"]))
		{
			return false;
		}

		auto& section_level = staff_resources_j["section_level"];
		auto& section_staff = staff_resources_j["section_staff"];

		auto& processed = this->staff_resources.resource_arrays[game::processed_server];
		auto& unprocessed = this->staff_resources.resource_arrays[game::unprocessed_server];
		auto& unit_levels = this->staff_resources.unit_levels;
		auto& unit_counts = this->staff_resources.unit_counts;

		processed[game::EMPLACEMENT_GUN_EAST] = placement.emplacement_gun_east;
		processed[game::EMPLACEMENT_GUN_WEST] = placement.emplacement_gun_west;
		processed[game::ANTI_AIR_GATLING_GUN_EAST] = placement.gatling_gun_east;
		processed[game::ANTI_AIR_GATLING_GUN_WEST] = placement.gatling_gun_west;
		processed[game::MORTAR_NORMAL] = placement.mortar_normal;

		processed[game::BIOTIC_RESOURCE] = usable_resource.biotic_resource;
		processed[game::COMMON_METAL] = usable_resource.common_metal;
		processed[game::FUEL_RESOURCE] = usable_resource.fuel_resource;
		processed[game::MINOR_METAL] = usable_resource.minor_metal;
		processed[game::PRECIOUS_METAL] = usable_resource.precious_metal;

		unprocessed[game::BIOTIC_RESOURCE] = processing_resource.biotic_resource;
		unprocessed[game::COMMON_METAL] = processing_resource.common_metal;
		unprocessed[game::FUEL_RESOURCE] = processing_resource.fuel_resource;
		unprocessed[game::MINOR_METAL] = processing_resource.minor_metal;
		unprocessed[game::PRECIOUS_METAL] = processing_resource.precious_metal;

		for (auto i = 0; i < game::unit_count; i++)
		{
			unit_levels.value[i] = section_level[game::unit_names[i]].get<std::uint32_t>();
		}

		auto total_staff = 0;
		for (auto i = 0ull; i < section_staff.size(); i++)
		{
			const auto& staff_of_rank_at_unit = section_staff[i];
			for (auto o = 0; o < game::unit_count; o++)
			{
				const auto& unit_name = game::unit_names[o];
				const auto& staff_count = staff_of_rank_at_unit[unit_name].get<std::uint32_t>();
				unit_counts.value[o] += staff_count;
				this->staff_resources.staff_count += staff_count;

				for (auto l = 0u; l < staff_count; l++)
				{
					auto staff_index = total_staff++;
					this->staff_resources.staff_array[staff_index].fields.header.peak_rank = i;
					this->staff_resources.staff_array[staff_index].fields.status_sync.designation = game::SECTION_COMBAT + o;
				}
			}
		}

		return true;
	}

	bool fob_events_settings_t::parse(nlohmann::json& data)
	{
		auto& event_duration_j = data["event_duration"];
		auto& event_players_j = data["event_players"];
		auto& event_list_j = data["event_list"];
		auto& event_rotation_j = data["event_rotation"];

		if (!event_duration_j.is_number_unsigned())
		{
			return false;
		}

		this->event_duration = event_duration_j.get<std::uint32_t>();

		for (auto i = 0ull; i < event_players_j.size(); i++)
		{
			auto player = std::make_shared<fob_event_player_t>();
			if (!player->parse(event_players_j[i]))
			{
				return false;
			}

			this->event_players.emplace_back(player);
		}

		for (auto i = 0ull; i < event_list_j.size(); i++)
		{
			auto event = std::make_shared<fob_event_t>();
			if (!event->parse(event_list_j[i]))
			{
				return false;
			}

			this->event_list.emplace_back(event);
		}

		for (auto i = 0ull; i < event_rotation_j.size(); i++)
		{
			auto& name_j = event_rotation_j[i];
			if (!name_j.is_string())
			{
				throw std::runtime_error(std::format("[fob events] invalid event rotation entry at index {}", i));
			}

			const auto name = name_j.get<std::string>();
			const auto iter = std::ranges::find_if(this->event_list.begin(), this->event_list.end(),
				[&](const std::shared_ptr<fob_event_t>& event)
				{
					return name == event->name;
				}
			);

			if (iter == this->event_list.end())
			{
				throw std::runtime_error(std::format("[fob events] event type \"{}\" not found", name));
			}

			this->event_rotation.emplace_back(*iter);
		}

		return true;
	}

	bool fob_event_t::parse(nlohmann::json& data)
	{
		auto& name_j = data["name"];
		auto& one_event_task_j = data["one_event_task"];
		auto& server_text_j = data["server_text"];
		auto& information_list_j = data["information_list"];
		auto& players_j = data["players"];
		auto& point_exchange_params_j = data["point_exchange_params"];

		if (!server_text_j.is_string() || !players_j.is_array() || !one_event_task_j.is_array())
		{
			return false;
		}

		this->name = name_j.get<std::string>();
		this->one_event_task = one_event_task_j;
		this->server_text = server_text_j.get<std::string>();

		if (!information_list_j.is_null())
		{
			for (auto i = 0ull; i < information_list_j.size(); i++)
			{
				online_news::information_data_t message{};

				if (!message.parse(information_list_j[i]))
				{
					continue;
				}

				message.info_id = static_cast<std::uint32_t>(i);
				this->information.emplace_back(message);
			}
		}

		if (!point_exchange_params_j.is_null())
		{
			if (!this->point_exchange_params.parse(data["point_exchange_params"]))
			{
				return false;
			}
		}

		for (auto i = 0ull; i < players_j.size(); i++)
		{
			const auto player_id = players_j[i].get<std::uint64_t>();
			this->player_ids.emplace_back(player_id);
		}

		return true;
	}

	std::shared_ptr<fob_event_player_t> fob_events_settings_t::get_player(const std::uint64_t player_id)
	{
		const auto iter = std::ranges::find_if(this->event_players.begin(), this->event_players.end(),
			[&](const std::shared_ptr<fob_event_player_t>& player)
			{
				return player->player_id == player_id;
			}
		);

		if (iter != this->event_players.end())
		{
			return *iter;
		}

		return nullptr;
	}

	bool is_event_text(const std::string& text)
	{
		const auto& event_data = get_event_settings();
		for (const auto& event : event_data.event_list)
		{
			for (const auto& message : event->information)
			{
				if (message.body == text)
				{
					return true;
				}
			}
		}

		return false;
	}

	GET_FIELD_C(fob_event, std::uint64_t, event_id);
	GET_FIELD_C(fob_event, std::string, event_type);
	GET_FIELD_C(fob_event, std::uint32_t, event_state);
	GET_FIELD_C(fob_event, std::chrono::seconds, start_date);
	GET_FIELD_C(fob_event, std::chrono::seconds, end_date);

	namespace impl
	{
		template <database_type_t Type>
		std::uint64_t create_event(database_t& db, const std::string& type,
			const std::chrono::system_clock::time_point& start_date, const std::chrono::system_clock::time_point& end_date)
		{
			auto result = db.exec<Type>(
				sqlpp::insert_into(fob_event::table)
					.set(fob_event::table.start_date = start_date,
						 fob_event::table.end_date = end_date,
						 fob_event::table.event_type = type));
			return result;
		}

		template <database_type_t Type>
		void set_event_state(database_t& db, const std::uint64_t event_id, const std::uint32_t state)
		{
			db.exec<Type>(
				sqlpp::update(fob_event::table)
					.set(fob_event::table.event_state = state)
						.where(fob_event::table.event_id == event_id));
		}

		template <database_type_t Type>
		std::optional<fob_event> get_current_event1(database_t& db)
		{
			auto results = db.exec<Type>(
				sqlpp::select(sqlpp::all_of(fob_event::table))
					.from(fob_event::table)
						.where(fob_event::table.event_state != static_cast<std::uint32_t>(event_state_dead)).limit(1u));


			std::optional<fob_event> event;

			if (results.empty())
			{
				return event;
			}

			event.emplace(results.front());
			return event;
		}

		template <database_type_t Type>
		std::optional<fob_event> get_current_event2()
		{
			std::optional<fob_event> event;

			database::access([&](database_t& db)
			{
				auto results = db.exec<Type>(
					sqlpp::select(sqlpp::all_of(fob_event::table))
						.from(fob_event::table)
							.where(fob_event::table.event_state != static_cast<std::uint32_t>(event_state_dead)).limit(1u));


				event.emplace(results.front());
			});

			return event;
		}
	}

	bool fob_event::initialize_params()
	{
		const auto& settings = get_event_settings();

		const auto iter = std::ranges::find_if(settings.event_list.begin(), settings.event_list.end(),
			[&](const std::shared_ptr<fob_event_t>& event)
			{
				return this->event_type_ == event->name;
			}
		);

		if (iter == settings.event_list.end())
		{
			return false;
		}

		this->params = *iter;
		return true;
	}

	std::optional<fob_event> get_current_event(database_t& db)
	{
		RUN_IMPL(impl::get_current_event1, db);
	}

	void set_event_state(database_t& db, const std::uint64_t event_id, const std::uint32_t state)
	{
		RUN_IMPL(impl::set_event_state, db, event_id, state);
	}

	std::optional<fob_event> get_current_event()
	{
		auto event = []()
		{
			RUN_IMPL(impl::get_current_event2);
		}();

		if (event.has_value() && !event->initialize_params())
		{
			return {};
		}

		return event;
	}

	std::uint64_t create_event(database_t& db, const std::string& type, const std::chrono::system_clock::time_point& start_date, const std::chrono::system_clock::time_point& end_date)
	{
		RUN_IMPL(impl::create_event, db, type, start_date, end_date);
	}

	void create_target_event(database_t& db)
	{
		const auto target_event = get_target_event();
		if (!target_event.has_value())
		{
			return;
		}

		create_event(db, target_event->params->name,
			std::chrono::system_clock::time_point(target_event->start),
			std::chrono::system_clock::time_point(target_event->end));

		console::print("[events] creating event instance for event type \"%s\"\n", target_event->params->name.data());
	}

	void add_event_news(database_t& db, const fob_event& event)
	{
		for (const auto& info : event.params->information)
		{
			online_news::add_info(info.body, false, false, event.get_end_date());
		}
	}

	void update_event(database_t& db)
	{
		auto current_event = get_current_event(db);
		if (!current_event.has_value() || !current_event->initialize_params())
		{
			create_target_event(db);
			return;
		}

		static std::chrono::system_clock::time_point last_ranking_update;
		static std::chrono::system_clock::time_point last_ranking_query;

		static std::size_t ranking_update_index = 0;
		static std::vector<std::uint8_t> ranking_entries =
		{
			event_rankings::most_ep_in_mission,
			event_rankings::cores_reached_defender,
			event_rankings::cores_reached_no_defender,
			event_rankings::damage_defender,
			event_rankings::materials_stolen,
			event_rankings::total_stealth,
			event_rankings::security_guards_neutralized,
			event_rankings::defense_deployments,
			event_rankings::defense_deployments_support,
			event_rankings::defense_deployments_support_received,
			event_rankings::defender_neutralized_intruder,
			event_rankings::successful_defenses,
			event_rankings::times_neutralized_by_intruder,
			event_rankings::fob_event_ranking,
			event_rankings::league_offsensive_boost,
			event_rankings::league_defensive_boost,
			event_rankings::league_grade_boost,
			event_rankings::league_security_boost,
			event_rankings::league_vehicles,
			event_rankings::league_fight_vehicles,
			event_rankings::league_walker_gears,
			event_rankings::league_emplacements,
			event_rankings::league_wins,
			event_rankings::league_pf_points_exchange,
			event_rankings::league_dispatch_mission_wins,
		};

		const auto now = std::chrono::system_clock::now();
		const auto now_s = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

		switch (current_event->get_event_state())
		{
		case event_state_none:
		{
			reset_values();
			add_event_news(db, current_event.value());
			set_event_state(db, current_event->get_event_id(), event_state_running);
			console::print("[events] event state changed: RUNNING\n");
			break;
		}
		case event_state_running:
		{
			if (now_s > current_event->get_end_date())
			{
				set_event_state(db, current_event->get_event_id(), event_state_finished);
				console::print("[events] event state changed: FINISHED\n");
				break;
			}

			const auto time_left = current_event->get_end_date() - now_s;
			if (ranking_update_index == 0 && time_left < event_intermission_duration)
			{
				set_event_state(db, current_event->get_event_id(), event_state_finished);
				console::print("[events] event state changed: FINISHED\n");
				break;
			}
			else
			{
				if ((ranking_update_index != 0 || now - last_ranking_update > 1h) && (now - last_ranking_query > 1min))
				{
					event_rankings::update_rankings(db, ranking_entries[ranking_update_index]);
					last_ranking_query = now;

					if (++ranking_update_index >= ranking_entries.size())
					{
						last_ranking_update = now;
						ranking_update_index = 0;
					}
				}
			}

			break;
		}
		case event_state_finished:
		{
			if (now_s >= current_event->get_end_date())
			{
				set_event_state(db, current_event->get_event_id(), event_state_dead);
				console::print("[events] event state changed: DEAD\n");
			}

			break;
		}
		case event_state_dead:
		{
			break;
		}
		}
	}

	bool is_intermission()
	{
		const auto event = get_current_event();
		if (!event.has_value())
		{
			return false;
		}

		return event->get_event_state() == event_state_finished;
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			database.run_query("mgstpp.fob_events.create");

			register_lang_vars();
			get_event_settings();
		}

		void run_tasks(database_t& database) override
		{
			update_event(database);
		}
	};
}

REGISTER_TABLE(database::fob_events::table, -1000)
