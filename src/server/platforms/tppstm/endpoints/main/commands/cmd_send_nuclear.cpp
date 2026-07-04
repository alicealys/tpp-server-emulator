#include <std_include.hpp>

#include "cmd_send_nuclear.hpp"

#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_send_nuclear::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto player_data = database::player_data::find(player->get_id());
		result["local_num"] = 0;
		result["server_num"] = player_data->get_nuke_count();

		return result;
	}
}
