#include <std_include.hpp>

#include "cmd_sync_soldier_bin.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"

#include <utils/cryptography.hpp>

namespace emulator::tpp
{
	nlohmann::json cmd_sync_soldier_bin::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto& soldier_num_j = data["soldier_num"];
		const auto& soldier_param_j = data["soldier_param"];
		const auto& version_j = data["version"];

		if (!version_j.is_number_unsigned() || !soldier_num_j.is_number_integer() || !soldier_param_j.is_string())
		{
			return error(ERR_INVALIDARG);
		}

		const auto soldier_num = soldier_num_j.get<std::uint32_t>();
		const auto soldier_param = soldier_param_j.get<std::string>();

		const auto& section = data["section"];
		const auto& section_soldier = data["section_soldier"];

		if (soldier_num > game::max_staff_count || !section.is_object() || section.size() != game::unit_count ||
			!section_soldier.is_object() || section_soldier.size() != game::unit_count)
		{
			return error(ERR_INVALIDARG);
		}

		std::string soldier_bin_resp;
		soldier_bin_resp.reserve(game::max_staff_count * 16ull);

		const auto local_version = version_j.get<std::uint32_t>();
		const auto client_version = player_data->get_client_staff_version();
		auto server_version = player_data->get_server_staff_version();

		const auto write_to_database = [&]
		{
			database::player_data::unit_levels_t levels{};
			database::player_data::unit_counts_t counts{};

			for (auto i = 0; i < game::unit_count; i++)
			{
				const auto& key = game::unit_names[i];
				if (section[key].is_number_integer())
				{
					levels[i] = section[key].get<std::uint32_t>();
				}

				if (section_soldier[key].is_number_integer())
				{
					counts[i] = section_soldier[key].get<std::uint32_t>();
				}
			}

			auto client_staff_array = database::player_data::staff_array_container::decode_client_staff_array(soldier_param);
			if (!client_staff_array.has_value())
			{
				result["error"] = game::get_error(ERR_INVALIDARG);
				return;
			}

			database::player_data::set_soldier_data(player->get_id(), soldier_num, client_staff_array.value(), levels, counts);
			database::player_data::sync_client_staff_version(player->get_id());

			result["soldier_num"] = soldier_num;
			result["soldier_param"] = client_staff_array->encode_client();
			result["version"] = server_version + 1;
		};

		const auto read_from_database = [&]
		{
			database::player_data::sync_client_staff_version(player->get_id());

			database::player_data::staff_array_container staff_array;
			player_data->get_staff_array(staff_array);

			result["soldier_num"] = player_data->get_staff_count();
			result["soldier_param"] = staff_array.encode_client();
			result["version"] = server_version;
		};

		if (client_version != server_version || local_version == client_version - 1)
		{
			read_from_database();
		}
		else
		{
			write_to_database();
		}

		return result;
	}
}
