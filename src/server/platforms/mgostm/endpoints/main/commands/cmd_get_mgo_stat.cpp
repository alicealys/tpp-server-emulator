#include <std_include.hpp>

#include "database/models/mgo_stats.hpp"

#include "cmd_get_mgo_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_get_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		if (!data["target"].is_object())
		{
			return error(ERR_INVALIDARG);
		}

		const auto target_player_opt = get_target_player(data["target"]);
		const auto target_player = target_player_opt.value_or(player.value());

		nlohmann::json result;
		
		auto& stat = result["stat"];

		stat["rule_stat_list"] = nlohmann::json::array();
		stat["stat_list"] = nlohmann::json::array();

		const auto stats = database::mgo_stats::get_stats(target_player.get_id());
		const auto& rules = database::mgo_stats::get_rules();
		const auto& stats_map = database::mgo_stats::get_stats_map();

		const auto get_stat_value = [&](const std::uint32_t id, const std::uint32_t rule_id)
		{
			const auto iter = std::ranges::find_if(stats, [&](auto& stat)
			{
				return stat.get_stat_id() == id && stat.get_rule_id() == rule_id;
			});

			return iter == stats.end() ? 0u : iter->get_stat_value();
		};

		auto index_rule = 0;
		auto index = 0;

		for (const auto& [id, info] : stats_map)
		{
			if (info.is_rule_specific)
			{
				stat["stat_list"][index]["id"] = id;
				stat["stat_list"][index]["value"] = get_stat_value(id, database::mgo_stats::rule_none);
				index++;
			}
			else
			{
				for (const auto& rule_id : rules)
				{
					stat["rule_stat_list"][index_rule]["id"] = id;
					stat["rule_stat_list"][index_rule]["rule_code"] = rule_id;
					stat["rule_stat_list"][index_rule]["value"] = get_stat_value(id, rule_id);
					index_rule++;
				}
			}
		}

		return result;
	}

	bool cmd_get_mgo_stat::needs_player()
	{
		return true;
	}
}
