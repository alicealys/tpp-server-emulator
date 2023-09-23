#include <std_include.hpp>

#include "cmd_get_player_platform_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/player_records.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_player_platform_list::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		if (!data["player_id_list"].is_array())
		{
			result["info_num"] = 32;
			for (auto i = 0; i < 32; i++)
			{
				result["info_list"][i]["npid"]["handler"]["data"] = "";
				result["info_list"][i]["npid"]["handler"]["dummy"] = {0, 0, 0};
				result["info_list"][i]["npid"]["handler"]["term"] = 0;
				result["info_list"][i]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
				result["info_list"][i]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
				result["info_list"][i]["player_id"] = 0;
				result["info_list"][i]["player_name"] = "NotImplement";
				result["info_list"][i]["ugc"] = 0;
				result["info_list"][i]["xuid"] = 0;
			}
		}
		else
		{
			auto index = 0;
			for (auto i = 0; i < data["player_id_list"].size(); i++)
			{
				const auto& value = data["player_id_list"][i];
				if (!value.is_number_unsigned())
				{
					continue;
				}

				const auto player_id = value.get<std::uint64_t>();
				const auto player = database::players::find(player_id);

				if (!player.has_value())
				{
					continue;
				}

				result["info_list"][index]["npid"]["handler"]["data"] = "";
				result["info_list"][index]["npid"]["handler"]["dummy"] = {0, 0, 0};
				result["info_list"][index]["npid"]["handler"]["term"] = 0;
				result["info_list"][index]["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
				result["info_list"][index]["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
				result["info_list"][index]["player_id"] = player->get_id();
				result["info_list"][index]["player_name"] = std::format("{}_player01", player->get_account_id());
				result["info_list"][index]["ugc"] = 0;
				result["info_list"][index]["xuid"] = player->get_account_id();
				++index;
			}

			result["info_num"] = index;
		}

		result["reward_count"] = 0;
		result["version"] = 0;
		result["reward_info"] = nlohmann::json::array();

		return result;
	}
}
