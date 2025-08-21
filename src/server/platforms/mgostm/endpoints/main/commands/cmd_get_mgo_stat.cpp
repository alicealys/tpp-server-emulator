#include <std_include.hpp>

#include "database/models/mgo_stats.hpp"

#include "cmd_get_mgo_stat.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_stat::cmd_get_mgo_stat()
	{
		this->rule_list_ = utils::resources::load_json(RESOURCE_MGO_RULE_STAT_LIST);
	}

	nlohmann::json cmd_get_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		
		auto& stat = result["stat"];

		stat["rule_stat_list"] = this->rule_list_;
		stat["stat_list"] = nlohmann::json::array();

		const auto stats = database::mgo_stats::get_stats(player->get_id());
		const auto stat_id_list = database::mgo_stats::get_id_list();

		for (auto i = 0ull; i < stat_id_list.size(); i++)
		{
			const auto id = stat_id_list[i];
			const auto value = std::ranges::find_if(stats, [&](auto& stat)
			{
				return stat.get_stat_id() == id;
			});

			stat["stat_list"][i]["id"] = id;
			stat["stat_list"][i]["value"] = value != stats.end() ? value->get_stat_value() : 0u;
		}

		return result;
	}

	bool cmd_get_mgo_stat::needs_player()
	{
		return true;
	}
}
