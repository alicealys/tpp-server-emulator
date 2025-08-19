#include <std_include.hpp>

#include "fob_event.hpp"
#include "players.hpp"
#include "player_data.hpp"
#include "player_records.hpp"
#include "fobs.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace database::fob_event
{
	namespace
	{
		constexpr auto account_id_start = 10000u;

		fob_event_date_range get_event_range()
		{
			const auto day = date::floor<date::days>(std::chrono::system_clock::now());
			const auto event_duration = std::chrono::days(7);
			const auto event_start = day - (date::weekday{day} - date::Tuesday);
			const auto event_end = event_start + event_duration;

			fob_event_date_range range{};
			range.start = std::chrono::duration_cast<std::chrono::seconds>(event_start.time_since_epoch());
			range.end = std::chrono::duration_cast<std::chrono::seconds>(event_end.time_since_epoch());

			return range;
		}

		fob_event_player parse_event_player(const nlohmann::json& player_j)
		{
			fob_event_player player{};

			player.emblem = player_j["emblem"];
			player.player_name = player_j["player_name"].get<std::string>();

			auto& fob_list = player_j["fob_list"];
			for (auto i = 0ull; i < fob_list.size(); i++)
			{
				player.fob_ids.emplace_back(fob_list[i]["mother_base_id"]);
				player.fobs.emplace_back(fob_list[i]);
			}

			auto& staff_resources = player_j["staff_resources"];

			{
				auto& placement = staff_resources["placement"];
				auto& usable_resource = staff_resources["usable_resource"];
				auto& processing_resource = staff_resources["processing_resource"];
				auto& section_level = staff_resources["section_level"];
				auto& section_staff = staff_resources["section_staff"];

				auto& processed = player.motherbase.resource_arrays[database::player_data::processed_server];
				auto& unprocessed = player.motherbase.resource_arrays[database::player_data::unprocessed_server];
				auto& unit_levels = player.motherbase.unit_levels;
				auto& unit_counts = player.motherbase.unit_counts;

				processed[database::player_data::emplacement_gun_east] = placement["emplacement_gun_east"].get<std::uint32_t>();
				processed[database::player_data::emplacement_gun_west] = placement["emplacement_gun_west"].get<std::uint32_t>();
				processed[database::player_data::gatling_gun_east] = placement["gatling_gun_east"].get<std::uint32_t>();
				processed[database::player_data::gatling_gun_west] = placement["gatling_gun_west"].get<std::uint32_t>();
				processed[database::player_data::mortar_normal] = placement["mortar_normal"].get<std::uint32_t>();

				processed[database::player_data::biotic_resource] = usable_resource["biotic_resource"].get<std::uint32_t>();
				processed[database::player_data::common_metal] = usable_resource["common_metal"].get<std::uint32_t>();
				processed[database::player_data::fuel_resource] = usable_resource["fuel_resource"].get<std::uint32_t>();
				processed[database::player_data::minor_metal] = usable_resource["minor_metal"].get<std::uint32_t>();
				processed[database::player_data::precious_metal] = usable_resource["precious_metal"].get<std::uint32_t>();

				unprocessed[database::player_data::biotic_resource] = processing_resource["biotic_resource"].get<std::uint32_t>();
				unprocessed[database::player_data::common_metal] = processing_resource["common_metal"].get<std::uint32_t>();
				unprocessed[database::player_data::fuel_resource] = processing_resource["fuel_resource"].get<std::uint32_t>();
				unprocessed[database::player_data::minor_metal] = processing_resource["minor_metal"].get<std::uint32_t>();
				unprocessed[database::player_data::precious_metal] = processing_resource["precious_metal"].get<std::uint32_t>();

				for (auto i = 0; i < database::player_data::unit_count; i++)
				{
					unit_levels[i] = section_level[database::player_data::unit_names[i]].get<std::uint32_t>();
				}
				
				player.motherbase.staff_array = utils::memory::allocate<database::player_data::staff_array_t>();
				auto staff_array = *player.motherbase.staff_array;

				auto total_staff = 0;
				for (auto i = 0ull; i < section_staff.size(); i++)
				{
					const auto& staff_of_rank_at_unit = section_staff[i];
					for (auto o = 0; o < database::player_data::unit_count; o++)
					{
						const auto& unit_name = database::player_data::unit_names[o];
						const auto& staff_count = staff_of_rank_at_unit[unit_name].get<std::uint32_t>();
						unit_counts[o] += staff_count;
						player.motherbase.staff_count += staff_count;

						for (auto l = 0u; l < staff_count; l++)
						{
							auto staff_index = total_staff++;
							staff_array[staff_index].fields.header.peak_rank = i;
							staff_array[staff_index].fields.status_sync.designation = database::player_data::des_units_start + o;
						}
					}
				}

				for (auto i = 0u; i < player.motherbase.staff_count; i++)
				{
					for (auto o = 0; o < 6; o++)
					{
						staff_array[i].packed[o] = BSWAP32(staff_array[i].packed[o]);
					}
				}
			}

			player.motherbase.motherbase = player_j["motherbase"];

			return player;
		}

		fob_event_t parse_event(const nlohmann::json& event_j)
		{
			fob_event_t event{};

			event.server_text = event_j["server_text"].get<std::string>();
			event.event_id = event_j["event_id"].get<std::uint32_t>();

			auto& information_list = event_j["information_list"];
			for (auto i = 0ull; i < information_list.size(); i++)
			{
				fob_event_information info{};
				info.important = information_list[i]["important"].get<std::string>();
				info.info_id = information_list[i]["info_id"].get<std::uint32_t>();
				info.mes_body = information_list[i]["mes_body"].get<std::string>();
				info.mes_subject = information_list[i]["mes_subject"].get<std::string>();
				event.information.emplace_back(info);
			}

			auto& point_exchange_params = event_j["point_exchange_params"];
			for (auto i = 0ull; i < point_exchange_params.size(); i++)
			{
				fob_event_point_exchange_param param{};
				param.common_value = point_exchange_params[i]["common_value"].get<std::uint32_t>();
				param.count = point_exchange_params[i]["count"].get<std::uint32_t>();
				param.exchange_limit = point_exchange_params[i]["exchange_limit"].get<std::uint32_t>();
				param.info_lang_id = point_exchange_params[i]["info_lang_id"].get<std::uint32_t>();
				param.limited_count = point_exchange_params[i]["limited_count"].get<std::uint32_t>();
				param.name_lang_id = point_exchange_params[i]["name_lang_id"].get<std::uint32_t>();
				param.point = point_exchange_params[i]["point"].get<std::uint32_t>();
				param.type = point_exchange_params[i]["type"].get<std::uint32_t>();
				param.unique_id = point_exchange_params[i]["unique_id"].get<std::uint32_t>();
				event.point_exchange_params.emplace_back(param);
			}

			auto& players = event_j["players"];
			for (auto i = 0ull; i < players.size(); i++)
			{
				event.players.emplace_back(parse_event_player(players[i]));
			}

			return event;
		}

		std::vector<fob_event_t> parse_event_list()
		{
			std::vector<fob_event_t> list;

			const auto list_j = utils::resources::load_json(RESOURCE_FOB_EVENT_LIST);
			for (auto i = 0ull; i < list_j["event_list"].size(); i++)
			{
				list.emplace_back(parse_event(list_j["event_list"][i]));
			}

			return list;
		}

		std::vector<fob_event_t>& get_event_list()
		{
			static auto event_list = parse_event_list();
			return event_list;
		}

		std::uint64_t account_id_from_event_id(const std::uint32_t event_id, const std::uint64_t player_index)
		{
			return account_id_start + event_id * 100 + player_index;
		}
		
		void create_database_entries_for_player(fob_event_t& event, fob_event_player& event_player, const std::size_t player_index)
		{
			const auto account_id = account_id_from_event_id(event.event_id, player_index);
			const auto player = database::players::find_or_insert(account_id, false);
			event_player.player_id = player.get_id();

			database::player_data::find_or_create(player.get_id());
			database::player_data::sync_emblem(player.get_id(), event_player.emblem);
			database::player_data::sync_motherbase(player.get_id(), event_player.motherbase.motherbase);
			database::player_data::set_resources(player.get_id(), event_player.motherbase.resource_arrays, 0, 0);

			database::player_data::set_soldier_data_raw(player.get_id(), event_player.motherbase.staff_count, event_player.motherbase.staff_array,
				event_player.motherbase.unit_levels, event_player.motherbase.unit_counts);

			database::player_records::find_or_create(player.get_id(), false);

			const auto fob_list = database::fobs::get_fob_list(player.get_id());
			if (fob_list.empty())
			{
				for (auto i = 0ull; i < event_player.fobs.size(); i++)
				{
					database::fobs::create(player.get_id(), 0u, event_player.fob_ids[i]);
				}
			}

			const auto new_fob_list = database::fobs::get_fob_list(player.get_id());
			database::fobs::sync_data(player.get_id(), event_player.fobs);
		}

		std::uint32_t get_week_number()
		{
			const auto day = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()).time_since_epoch().count();
			const auto week = day / 7;
			return week;
		}

		std::size_t get_current_event_index()
		{
			const auto week = get_week_number();
			auto& event_list = get_event_list();
			const auto index = week % event_list.size();
			return index;
		}

		void create_database_entries()
		{
			auto& event_list = get_event_list();
			for (auto& event : event_list)
			{
				for (auto i = 0ull; i < event.players.size(); i++)
				{
					create_database_entries_for_player(event, event.players[i], i);
				}
			}
		}
	}

	std::optional<fob_event_t> get_current_event()
	{
		auto& event_list = get_event_list();
		if (event_list.empty())
		{
			return {};
		}

		const auto index = get_current_event_index();
		auto& event = event_list[index];
		event.date_range = get_event_range();
		return event;
	}

	class table final : public table_interface
	{
	public:
		void create(database_t& database) override
		{
			get_current_event_index();
			create_database_entries();
		}
	};
}

REGISTER_TABLE(database::fob_event::table, -1000)
