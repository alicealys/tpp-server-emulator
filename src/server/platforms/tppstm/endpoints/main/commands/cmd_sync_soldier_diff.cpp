#include <std_include.hpp>

#include "cmd_sync_soldier_diff.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"

#include <utils/nt.hpp>
#include <utils/cryptography.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_soldier_diff::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto p_data = database::player_data::find(player->get_id());
		if (!p_data.get())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& section = data["section"];
		const auto& section_soldier = data["section_soldier"];

		if (!section.is_object() || section.size() != database::player_data::unit_count ||
			!section_soldier.is_object() || section_soldier.size() != database::player_data::unit_count)
		{
			return error(ERR_INVALIDARG);
		}

		auto& remover_param = data["remover_param"];

		std::string new_staff_array;
		std::unordered_set<std::uint32_t> staff_indices;
		auto update_staff = false;

		if (remover_param.is_array())
		{
			std::vector<database::player_data::staff_t> remover_params;

			for (auto i = 0; i < remover_param.size(); i++)
			{
				const auto& param = remover_param[i]["param"];
				if (!param.is_array() || param.size() != 2 ||
					!param[0].is_number_unsigned() || !param[1].is_number_unsigned())
				{
					continue;
				}

				const auto header = param[0].get<std::uint32_t>();
				const auto seed = param[1].get<std::uint32_t>();

				database::player_data::staff_t staff{};
				staff.fields.packed_header = header;
				staff.fields.packed_seed = seed;
				remover_params.emplace_back(staff);
			}

			for (auto i = 0u; i < p_data->get_staff_count(); i++)
			{
				auto removed = false;
				const auto staff = p_data->get_staff(i);

				for (const auto& remover : remover_params)
				{
					if (staff.packed_header == remover.fields.packed_header && 
						staff.packed_seed == remover.fields.packed_seed)
					{
						removed = true;
						break;
					}
				}

				if (!removed)
				{
					staff_indices.insert(i);
				}
				else
				{
					update_staff = true;
				}
			}
		}

		if (update_staff)
		{
			new_staff_array.reserve(staff_indices.size() * sizeof(database::player_data::staff_t));

			for (const auto& idx : staff_indices)
			{
				const auto staff = p_data->get_staff(idx);
				std::uint32_t params[6]{};
				params[0] = _byteswap_ulong(staff.unk.data);
				params[1] = _byteswap_ulong(staff.unk2.data);
				params[2] = _byteswap_ulong(staff.packed_header);
				params[3] = _byteswap_ulong(staff.packed_seed);
				params[4] = _byteswap_ulong(staff.packed_status_sync);
				params[5] = _byteswap_ulong(staff.packed_status_no_sync);
				new_staff_array.append(reinterpret_cast<char*>(params), sizeof(params));
			}
		}

		while (new_staff_array.size() < sizeof(database::player_data::staff_array_t))
		{
			new_staff_array += '\0';
		}

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

		if (update_staff)
		{
			database::player_data::set_soldier_data(player->get_id(), 
				static_cast<std::uint32_t>(staff_indices.size()), 
				new_staff_array, levels, counts);
		}
		else
		{
			database::player_data::set_soldier_diff(player->get_id(), levels, counts);
		}

		result["result"] = "NOERR";

		return result;
	}
}
