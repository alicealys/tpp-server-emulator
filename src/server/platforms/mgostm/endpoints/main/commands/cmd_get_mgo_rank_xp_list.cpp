#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_get_mgo_rank_xp_list.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_rank_xp_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& target_list = data["target_list"];
		if (!target_list.is_array())
		{
			return error(ERR_INVALIDARG);
		}

		result["result_list"] = nlohmann::json::array();

		for (auto i = 0ull; i < target_list.size(); i++)
		{
			const auto target_player = get_target_player(target_list[i]);

			result["result_list"][i]["rank_xp"] = 0;
			result["result_list"][i]["result"] = game::get_error(NOERR);

			if (target_player.has_value())
			{
				const auto mgo_data = database::mgo_data::find(target_player->get_id());
				if (mgo_data.has_value())
				{
					result["result_list"][i]["rank_xp"] = mgo_data->get_rank_xp();
				}
			}
		}

		return result;
	}
}
