#include <std_include.hpp>

#include "fob_events.hpp"
#include "players.hpp"
#include "player_data.hpp"
#include "player_records.hpp"
#include "fobs.hpp"
#include "variables.hpp"
#include "event_rankings.hpp"
#include "mgo_data.hpp"
#include "../../component/command.hpp"

#include "utils/lang_strings.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::fob_events
{
	namespace
	{
		constexpr auto event_number_variable_name = "fob_event_number";

		std::uint32_t get_information_message_id(const emulator::tpp::cmd_get_informationlist2_base::message_t& message)
		{
			const auto& event = get_current_event();
			if (!event.has_value())
			{
				return 0u;
			}

			return 1000000u + event->index * 10u + (event->number) * 20u + message.info_id;
		}

		std::uint64_t get_information_message_date(const emulator::tpp::cmd_get_informationlist2_base::message_t& message)
		{
			const auto& event = get_current_event();
			if (!event.has_value())
			{
				return 0u;
			}

			return event->start.count();
		}

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

				return date::format("%b. %d, %Y", std::chrono::system_clock::time_point(current_event->start));
			});

			utils::lang::register_var("event_end", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%b. %d, %Y", std::chrono::system_clock::time_point(current_event->end));
			});

			utils::lang::register_var("event_start_ymd", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%Y/%m/%d", std::chrono::system_clock::time_point(current_event->start));
			});

			utils::lang::register_var("event_end_ymd", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%Y/%m/%d", std::chrono::system_clock::time_point(current_event->end));
			});

			utils::lang::register_var("event_start_ymd_rev", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%d/%m/%Y", std::chrono::system_clock::time_point(current_event->start));
			});

			utils::lang::register_var("event_end_ymd_rev", [](const std::uint8_t)
				-> std::string
			{
				const auto current_event = database::fob_events::get_current_event();
				if (!current_event.has_value())
				{
					return {};
				}

				return date::format("%d/%m/%Y", std::chrono::system_clock::time_point(current_event->end));
			});
		}

		std::uint32_t get_event_number()
		{
			const auto now = std::chrono::system_clock::now() + 24h * 7;
			constexpr auto offset = 24h * 5 + 5h; // tuesday 5AM
			const auto day = std::chrono::floor<std::chrono::days>(now - offset).time_since_epoch().count();
			const auto week = day / 7;
			return week;
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

		fob_events_data_t parse_events_data()
		{
			fob_events_data_t event_data{};

			auto data = utils::resources::load_json(RESOURCE_FOB_EVENT_LIST);
			if (!event_data.parse(data))
			{
				throw std::runtime_error("[fob events] failed to parse fob event list");
			}

			for (auto& player : event_data.players)
			{
				create_database_entries_for_player(*player);
			}

			for (auto& event : event_data.events)
			{
				for (auto& player_id : event->player_ids)
				{
					const auto player = event_data.get_player(player_id);
					if (player == nullptr)
					{
						throw std::runtime_error(std::format("[fob events] invalid player id {}", player_id));
					}

					event->players.emplace_back(player);
				}
			}

			return event_data;
		}

		const fob_events_data_t& get_events_data()
		{
			static const auto event_list = parse_events_data();
			return event_list;
		}

		std::size_t get_current_event_index()
		{
			const auto week = get_event_number();
			auto& events_data = get_events_data();
			const auto index = week % events_data.events.size();
			return index;
		}

		void reset_values()
		{
			database::event_rankings::reset_periodic_values();
			database::player_records::reset_event_points();
			database::mgo_data::reset_survival_tickets(database::mgo_data::base_survival_tickets);
		}

		void update_event()
		{
			static auto last_update = std::chrono::high_resolution_clock::time_point();
			const auto now = std::chrono::high_resolution_clock::now();

			if (now - last_update < 1h)
			{
				return;
			}

			last_update = now;

			const auto current_event_number = get_event_number();
			const auto stored_event_number = variables::get(event_number_variable_name);

			if (!stored_event_number.has_value() || !stored_event_number->is_number_unsigned())
			{
				reset_values();
				variables::set(event_number_variable_name, current_event_number);
				return;
			}

			const auto number = stored_event_number->get<std::uint32_t>();
			if (number == current_event_number)
			{
				return;
			}

			reset_values();
			variables::set(event_number_variable_name, current_event_number);
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

	bool fob_events_data_t::parse(nlohmann::json& data)
	{
		auto& players_j = data["players"];
		auto& event_list_j = data["event_list"];

		for (auto i = 0ull; i < players_j.size(); i++)
		{
			auto player = std::make_shared<fob_event_player_t>();
			if (!player->parse(players_j[i]))
			{
				return false;
			}

			this->players.emplace_back(player);
		}

		for (auto i = 0ull; i < event_list_j.size(); i++)
		{
			auto event = std::make_shared<fob_event_t>();
			if (!event->parse(event_list_j[i]))
			{
				return false;
			}

			this->events.emplace_back(event);
		}

		return true;
	}

	bool fob_event_t::parse(nlohmann::json& data)
	{
		this->one_event_task = data["one_event_task"];
		this->server_text = data["server_text"].get<std::string>();

		auto& information_list = data["information_list"];
		for (auto i = 0ull; i < information_list.size(); i++)
		{
			emulator::tpp::cmd_get_informationlist2_base::message_t message{};

			if (!message.parse(information_list[i]))
			{
				continue;
			}

			message.get_info_id = get_information_message_id;
			message.get_date = get_information_message_date;
			message.info_id = static_cast<std::uint32_t>(i);
			this->information.emplace_back(message);
		}

		if (!this->point_exchange_params.parse(data["point_exchange_params"]))
		{
			return false;
		}

		auto& players_j = data["players"];
		for (auto i = 0ull; i < players_j.size(); i++)
		{
			const auto player_id = players_j[i].get<std::uint64_t>();
			this->player_ids.emplace_back(player_id);
		}

		return true;
	}

	void get_maintenance_range(std::chrono::system_clock::time_point& start, std::chrono::system_clock::time_point& end)
	{
		const auto day = date::floor<date::days>(std::chrono::system_clock::now());
		const auto event_duration = std::chrono::days(7);
		start = (day - (date::weekday{day} - date::Tuesday)) + std::chrono::hours(5);
		end = start + event_duration;

		if (std::chrono::system_clock::now() < start)
		{
			start -= event_duration;
			end -= event_duration;
		}
	}

	void get_event_range(std::chrono::seconds& start, std::chrono::seconds& end)
	{
		std::chrono::system_clock::time_point event_start{};
		std::chrono::system_clock::time_point event_end{};
		get_maintenance_range(event_start, event_end);

		start = std::chrono::duration_cast<std::chrono::seconds>(event_start.time_since_epoch());
		end = std::chrono::duration_cast<std::chrono::seconds>(event_end.time_since_epoch());
	}

	std::shared_ptr<fob_event_player_t> fob_events_data_t::get_player(const std::uint64_t player_id)
	{
		const auto iter = std::ranges::find_if(this->players.begin(), this->players.end(),
			[&](const std::shared_ptr<fob_event_player_t>& player)
			{
				return player->player_id == player_id;
			}
		);

		if (iter != this->players.end())
		{
			return *iter;
		}

		return nullptr;
	}

	std::optional<running_event_info_t> get_current_event()
	{
		running_event_info_t info{};

		const auto& events_data = get_events_data();
		if (events_data.events.empty())
		{
			return {};
		}

		const auto index = get_current_event_index();

		info.index = static_cast<std::uint32_t>(index);
		info.number = get_event_number();
		info.params = events_data.events[index];
		get_event_range(info.start, info.end);

		return {info};
	}

	bool is_intermission()
	{
		std::chrono::system_clock::time_point event_start{};
		std::chrono::system_clock::time_point event_end{};
		get_maintenance_range(event_start, event_end);
		const auto now = std::chrono::system_clock::now();
		return now < event_end && now > event_end - event_intermission_duration;
	}

	bool is_event_text(const std::string& text)
	{
		const auto& event_data = get_events_data();
		for (const auto& event : event_data.events)
		{
			for (const auto& message : event->information)
			{
				if (message.text == text)
				{
					return true;
				}
			}
		}

		return false;
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			register_lang_vars();
			get_events_data();

			command::add("get_current_event", []()
			{
				const auto event = get_current_event();
				if (event.has_value())
				{
					console::print("[FOB Events] current event: \n");
					console::print("\tname: %s\n", event->params->server_text.data());
					console::print("\tplayer list:\n");

					for (const auto& player : event->params->players)
					{
						if (player != nullptr)
						{
							console::print("\t\tid: %i, name: \"%s\"\n", player->player_id, player->player_name.data());
						}
					}

					console::print("--------------------------------------------\n");
				}
				else
				{
					console::print("[FOB Events] no events running\n");
				}
			});

			command::execute("get_current_event");
		}

		void run_tasks(database_t& database) override
		{
			update_event();
		}
	};
}

REGISTER_TABLE(database::fob_events::table, -1000)
