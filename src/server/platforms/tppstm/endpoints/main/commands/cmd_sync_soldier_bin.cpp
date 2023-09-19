#include <std_include.hpp>

#include "cmd_sync_soldier_bin.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"

#include <utils/nt.hpp>
#include <utils/cryptography.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_soldier_bin::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto& soldier_num_val = data["soldier_num"];
		const auto& soldier_param_val = data["soldier_param"];

		if (!soldier_num_val.is_number_integer() || !soldier_param_val.is_string())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto soldier_param = soldier_param_val.get<std::string>();
		auto soldier_bin = utils::cryptography::base64::decode(utils::encoding::decode_url_string(soldier_param));

		if (soldier_bin.empty())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		if (soldier_bin.size() > sizeof(database::player_data::staff_array_t) || (soldier_bin.size() % 24) != 0)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto& section = data["section"];
		const auto& section_soldier = data["section_soldier"];

		if (!section.is_object() || section.size() != database::player_data::unit_count ||
			!section_soldier.is_object() || section_soldier.size() != database::player_data::unit_count)
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
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

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto soldier_count = static_cast<std::uint32_t>(soldier_bin.size() / 24);
		database::player_data::set_soldier_data(player->get_id(), soldier_count, soldier_bin, levels, counts);

		std::string soldier_bin_resp;
		soldier_bin_resp.reserve(soldier_count * 16);
		for (auto i = 0u; i < soldier_count; i++)
		{
			soldier_bin_resp.append(&soldier_bin[i * 24 + 8], 16);
		}

		result["result"] = "NOERR";
		result["flag"] = "SYNC";
		result["force_sync"] = 0;
		result["soldier_num"] = soldier_count;
		result["soldier_param"] = utils::cryptography::base64::encode(soldier_bin_resp);

		return result;
	}
}
