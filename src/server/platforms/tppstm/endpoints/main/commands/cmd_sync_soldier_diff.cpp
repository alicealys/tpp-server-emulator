#include <std_include.hpp>

#include "cmd_sync_soldier_diff.hpp"

#include "database/models/player_data.hpp"
#include "database/models/players.hpp"

#include "utils/encoding.hpp"

#include <utils/nt.hpp>
#include <utils/cryptography.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_soldier_diff::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto p_data = database::player_data::find(player->get_id());
		if (!p_data.get())
		{
			printf("pdata not fnd\n");

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

		database::player_data::set_soldier_diff(player->get_id(), levels, counts);

		result["result"] = "NOERR";

		return result;
	}
}
