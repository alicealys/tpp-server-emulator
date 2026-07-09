#include <std_include.hpp>

#include "database/models/player_data.hpp"
#include "database/models/variables.hpp"

#include "cmd_destruct_online_nuclear.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_destruct_online_nuclear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& count_j = data["count"];
		if (!count_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		database::player_data::resource_arrays_t resources{};
		database::player_data::get_resource_arrays(player->get_id(), resources);

		const auto nuke_count = resources[game::processed_server][game::NUCLEAR_WEAPON];
		const auto destruct_count = std::min(nuke_count, count_j.get<std::uint32_t>());

		resources[game::processed_local][game::NUCLEAR_WEAPON] = 0u;
		resources[game::processed_server][game::NUCLEAR_WEAPON] = nuke_count - destruct_count;

		database::player_data::set_resources(player->get_id(), resources);
		database::player_data::inc_nuke_destruct_count(player->get_id(), destruct_count);

		const auto total_nukes = database::player_data::get_nuke_count();
		if (total_nukes == 0)
		{
			const auto count = database::variables::get<std::uint32_t>("abolition_count", 0u);
			database::variables::set("abolition_count", count + 1);
			database::variables::set("abolition_date", std::time(nullptr));
		}

		return result;
	}

	bool cmd_destruct_online_nuclear::needs_player()
	{
		return true;
	}
}
