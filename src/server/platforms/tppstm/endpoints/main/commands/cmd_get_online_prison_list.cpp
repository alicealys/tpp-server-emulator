#include <std_include.hpp>

#include "cmd_get_online_prison_list.hpp"

#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_online_prison_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& is_persuade_j = data["is_persuade"];
		auto& num_j = data["num"];
		auto& offset_j = data["offset"];

		if (!is_persuade_j.is_number_unsigned() || !num_j.is_number_unsigned() || !offset_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto is_persuade = is_persuade_j.get<std::uint32_t>() == 1;
		const auto num = num_j.get<std::uint32_t>();
		const auto offset = offset_j.get<std::uint32_t>();

		const auto player_data = database::player_data::find(player->get_id());

		result["prison_soldier_param"] = nlohmann::json::array();
		result["rescue_list"] = nlohmann::json::array();
		result["rescue_num"] = 0;
		result["soldier_num"] = 0;
		result["total_num"] = 0;

		if (!player_data.has_value())
		{
			return result;
		}

		database::player_data::prisoner_array_container prison;
		player_data->get_prisoner_array(prison);

		std::optional<database::player_data::prisoner_array_container> new_prison;
		if (is_persuade)
		{
			new_prison.emplace(database::player_data::prisoner_array_container{});
		}

		auto idx = 0u;
		auto total_count = 0u;
		auto new_prison_idx = 0u;

		for (auto i = 0u; i < prison.size(); i++)
		{
			if (prison[i].owner_id == 0)
			{
				continue;
			}

			const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			const auto percent = static_cast<float>(now - prison[i].time_captured) / static_cast<float>(database::player_data::prisoner_hold_time.count());
			const auto is_persuaded = percent >= 1.f;

			if (!is_persuaded)
			{
				++total_count;
			}

			if (i >= offset && idx < num && is_persuaded == is_persuade)
			{
				auto& prisoner_j = result["prison_soldier_param"][idx++];
				prisoner_j["param"][0] = prison[i].data.fields.packed_header;
				prisoner_j["param"][1] = prison[i].data.fields.packed_seed;
				prisoner_j["param"][2] = prison[i].data.fields.packed_status_no_sync;
				prisoner_j["param"][3] = prison[i].data.fields.packed_status_sync;
				prisoner_j["param"][4] = 15 - static_cast<int>(percent * 15);
			}

			if (new_prison.has_value() && !is_persuaded)
			{
				new_prison->operator[](new_prison_idx++) = prison[i];
			}
		}

		if (new_prison.has_value())
		{
			database::player_data::set_prison_bin(player->get_id(), new_prison.value());
		}

		result["soldier_num"] = idx;
		result["total_num"] = total_count;

		return result;
	}
}
