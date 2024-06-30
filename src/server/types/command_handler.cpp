#include <std_include.hpp>

#include "command_handler.hpp"

namespace tpp
{
	nlohmann::json error(const std::string& id)
	{
		nlohmann::json result;
		result["result"] = id;
		return result;
	}

	nlohmann::json error(const std::uint32_t id)
	{
		nlohmann::json result;
		result["result"] = utils::tpp::get_error(id);
		return result;
	}

	nlohmann::json resource(const std::uint32_t id)
	{
		auto resource = utils::resources::load_json(id);
		resource["result"] = utils::tpp::get_error(NOERR);
		return resource;
	}

	nlohmann::json player_info(const std::uint64_t player_id, const std::uint64_t account_id)
	{
		nlohmann::json info;

		info["npid"]["handler"]["data"] = "";
		info["npid"]["handler"]["dummy"] = {0, 0, 0};
		info["npid"]["handler"]["term"] = 0;
		info["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
		info["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
		info["player_id"] = player_id;
		info["player_name"] = account_id == 0 ? "NotImplement" : std::format("{}_player01", account_id);
		info["ugc"] = 1;
		info["xuid"] = account_id;

		return info;
	}

	nlohmann::json player_info(const database::players::player& player)
	{
		return player_info(player.get_id(), player.get_account_id());
	}

	nlohmann::json player_info(const std::optional<database::players::player>& player)
	{
		return player_info(*player);
	}
}
