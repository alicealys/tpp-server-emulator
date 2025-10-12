#include <std_include.hpp>

#include "database/models/mgo_stats.hpp"

#include "cmd_set_mgo_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& stat = data["stat"];
		if (!stat.is_object() || !stat["stat_list"].is_array() || !stat["rule_stat_list"].is_array())
		{
			return error(ERR_INVALIDARG);
		}

		auto& list = stat["stat_list"];
		auto& rule_list = stat["rule_stat_list"];

		const auto& rules = database::mgo_stats::get_rules();
		const auto& stats_map = database::mgo_stats::get_stats_map();

		for (auto i = 0ull; i < rule_list.size(); i++)
		{
			const auto& id_j = rule_list[i]["id"];
			const auto& rule_code_j = rule_list[i]["rule_code"];
			const auto& value_j = rule_list[i]["value"];

			if (!id_j.is_number_unsigned() || !rule_code_j.is_number_unsigned() || !value_j.is_number_unsigned())
			{
				continue;
			}

			const auto id = id_j.get<std::uint32_t>();
			const auto rule_code = rule_code_j.get<std::uint32_t>();
			const auto value = value_j.get<std::uint32_t>();

			const auto iter = stats_map.find(id);
			if (iter == stats_map.end() || iter->second.is_rule_specific || !rules.contains(rule_code))
			{
				continue;
			}

			database::mgo_stats::set_stat(player->get_id(), rule_code, id, value);
		}

		for (auto i = 0ull; i < list.size(); i++)
		{
			const auto& id_j = list[i]["id"];
			const auto& value_j = list[i]["value"];

			if (!id_j.is_number_unsigned() || !value_j.is_number_unsigned())
			{
				continue;
			}

			const auto id = id_j.get<std::uint32_t>();
			const auto value = value_j.get<std::uint32_t>();

			const auto iter = stats_map.find(id);
			if (iter == stats_map.end() || !iter->second.is_rule_specific)
			{
				continue;
			}

			database::mgo_stats::set_stat(player->get_id(), database::mgo_stats::rule_none, id, value);
		}

		return result;
	}

	bool cmd_set_mgo_stat::needs_player()
	{
		return true;
	}
}
