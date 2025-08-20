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
		if (!player_data.get())
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

		auto soldier_num = soldier_num_j.get<std::uint32_t>();
		const auto soldier_param = soldier_param_j.get<std::string>();

		auto soldier_bin = utils::cryptography::base64::decode(utils::encoding::decode_url_string(soldier_param));

		if (soldier_bin.empty())
		{
			return error(ERR_INVALIDARG);
		}

		if (soldier_bin.size() != sizeof(database::player_data::staff_array_t))
		{
			return error(ERR_INVALIDARG);
		}

		const auto& section = data["section"];
		const auto& section_soldier = data["section_soldier"];

		if (soldier_num > database::player_data::max_staff_count || !section.is_object() || section.size() != database::player_data::unit_count ||
			!section_soldier.is_object() || section_soldier.size() != database::player_data::unit_count)
		{
			return error(ERR_INVALIDARG);
		}

		std::string soldier_bin_resp;
		soldier_bin_resp.reserve(database::player_data::max_staff_count * 16ull);

		const auto local_version = version_j.get<std::uint32_t>();
		const auto client_version = player_data->get_client_staff_version();
		auto server_version = player_data->get_server_staff_version();

		const auto write_to_database = [&]
		{
			database::player_data::unit_levels_t levels{};
			database::player_data::unit_counts_t counts{};

			for (auto i = 0; i < database::player_data::unit_count; i++)
			{
				const auto& key = database::player_data::unit_names[i];
				if (section[key].is_number_integer())
				{
					levels[i] = section[key].get<std::uint32_t>();
				}

				if (section_soldier[key].is_number_integer())
				{
					counts[i] = section_soldier[key].get<std::uint32_t>();
				}
			}

			database::player_data::set_soldier_data(player->get_id(), soldier_num, soldier_bin, levels, counts);

			for (auto i = 0u; i < database::player_data::max_staff_count; i++)
			{
				soldier_bin_resp.append(&soldier_bin[i * 24ull + 8], 16);
			}

			server_version++;
		};

		const auto read_from_database = [&]
		{
			std::string current_soldier_bin;
			current_soldier_bin.resize(sizeof(database::player_data::staff_array_t));
			auto staff_array = reinterpret_cast<database::player_data::staff_t*>(current_soldier_bin.data());
			player_data->copy_staff_array(staff_array);
			database::player_data::reverse_staff_array_bytes(staff_array);

			soldier_num = player_data->get_staff_count();

			for (auto i = 0u; i < database::player_data::max_staff_count; i++)
			{
				soldier_bin_resp.append(&current_soldier_bin[i * 24ull + 8], 16);
			}
		};

		if (client_version != server_version || local_version == client_version - 1)
		{
			read_from_database();
		}
		else
		{
			write_to_database();
		}

		database::player_data::sync_client_staff_version(player->get_id());

		result["result"] = "NOERR";
		result["soldier_num"] = soldier_num;
		result["soldier_param"] = utils::cryptography::base64::encode(soldier_bin_resp);
		result["version"] = server_version;

		return result;
	}
}
