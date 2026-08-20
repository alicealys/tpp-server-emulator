#include <std_include.hpp>

#include "cmd_get_mgo_title_usr.hpp"

#include "database/models/mgo_titles.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_title_usr::generate(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		if (!data["target"].is_object())
		{
			return error(ERR_INVALIDARG);
		}

		auto has_id = false;
		auto target_player_opt = get_target_player(data["target"], &has_id);
		if (!has_id)
		{
			target_player_opt = player;
		}

		result["title_list"] = nlohmann::json::array();

		if (!target_player_opt.has_value())
		{
			return result;
		}

		const auto player_titles = database::mgo_titles::get_player_title_list(target_player_opt->get_id());
		for (auto i = 0ull; i < player_titles.size(); i++)
		{
			auto& entry_j = result["title_list"][i];
			entry_j["flag"] = player_titles[i].get_flag();
			entry_j["gp"] = player_titles[i].get_gp();
			entry_j["id"] = player_titles[i].get_title_id();
		}

		return result;
	}

	nlohmann::json cmd_get_mgo_title_usr::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return cmd_get_mgo_title_usr::generate(data, player);
	}

	bool cmd_get_mgo_title_usr::needs_player()
	{
		return true;
	}
}
