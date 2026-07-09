#include <std_include.hpp>

#include "cmd_get_contribute_player_list.hpp"

#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_contribute_player_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& num_j = data["num"];
		if (!num_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto limit = std::min(50u, num_j.get<std::uint32_t>());
		const auto list = database::player_data::get_nuclear_abolition_contributors(limit);

		result["info_list"] = nlohmann::json::array();
		result["info_num"] = list.size();

		for (auto i = 0u; i < list.size(); i++)
		{
			result["info_list"][i] = player_info(list[i]);
		}

		return result;
	}
}
