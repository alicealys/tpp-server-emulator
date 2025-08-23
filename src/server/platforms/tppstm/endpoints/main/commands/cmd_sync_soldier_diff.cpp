#include <std_include.hpp>

#include "cmd_sync_soldier_diff.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"

#include <utils/cryptography.hpp>

namespace emulator::tpp
{
	namespace
	{
		struct staff_diff_t
		{
			bool removed;
			database::player_data::staff_t staff;
		};
	}

	nlohmann::json cmd_sync_soldier_diff::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& section = data["section"];
		const auto& section_soldier = data["section_soldier"];
		const auto& client_version_j = data["version"];

		if (!section.is_object() || section.size() != database::player_data::unit_count ||
			!section_soldier.is_object() || section_soldier.size() != database::player_data::unit_count ||
			!client_version_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
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

		auto& remover_param = data["remover_param"];
		auto& soldier_param = data["soldier_param"];

		std::vector<staff_diff_t> staff_diffs;

		const auto validate_param = [](const nlohmann::json& param, const std::size_t size)
		{
			if (!param.is_array() || param.size() != size)
			{
				return false;
			}

			for (auto i = 0ull; i < size; i++)
			{
				if (!param[i].is_number_unsigned())
				{
					return false;
				}
			}

			return true;
		};

		auto update_staff = false;

		if (remover_param.is_array())
		{
			update_staff = true;
			for (auto i = 0ull; i < remover_param.size(); i++)
			{
				const auto& param = remover_param[i]["param"];
				if (!validate_param(param, 2ull))
				{
					continue;
				}

				staff_diff_t diff{};
				diff.removed = true;
				diff.staff.fields.packed_header = param[0].get<std::uint32_t>();;
				diff.staff.fields.packed_seed = param[1].get<std::uint32_t>();;
				staff_diffs.emplace_back(diff);
			}
		}

		if (soldier_param.is_array())
		{
			update_staff = true;
			for (auto i = 0ull; i < soldier_param.size(); i++)
			{
				const auto& param = soldier_param[i]["param"];
				if (!validate_param(param, 6ull))
				{
					continue;
				}

				staff_diff_t diff{};
				diff.removed = false;

				for (auto o = 0; o < 6; o++)
				{
					diff.staff.packed[o] = param[o].get<std::uint32_t>();
				}

				staff_diffs.emplace_back(diff);
			}
		}

		if (update_staff)
		{
			auto staff_index = 0u;
			database::player_data::staff_array_container old_staff_array;
			database::player_data::staff_array_container new_staff_array;

			player_data->get_staff_array(old_staff_array);

			for (auto i = 0u; i < database::player_data::max_staff_count; i++)
			{
				const auto& old_staff = old_staff_array[i];
				if (old_staff.fields.packed_seed == 0)
				{
					continue;
				}

				const auto iter = std::find_if(staff_diffs.begin(), staff_diffs.end(), [&](staff_diff_t& diff)
				{
					return old_staff.fields.packed_header == diff.staff.fields.packed_header && old_staff.fields.packed_seed == diff.staff.fields.packed_seed;
				});

				if (iter == staff_diffs.end())
				{
					new_staff_array[staff_index++] = old_staff;
					continue;
				}

				if (iter->removed)
				{
					staff_diffs.erase(iter);
					continue;
				}

				new_staff_array[staff_index++].fields = iter->staff.fields;
				staff_diffs.erase(iter);
			}

			database::player_data::set_soldier_data(player->get_id(), staff_index, new_staff_array, levels, counts);
		}
		else
		{
			database::player_data::set_soldier_diff(player->get_id(), levels, counts);
		}
		
		database::player_data::sync_client_staff_version(player->get_id());

		result["result"] = "NOERR";
		result["version"] = player_data->get_server_staff_version() + 1;

		return result;
	}
}
