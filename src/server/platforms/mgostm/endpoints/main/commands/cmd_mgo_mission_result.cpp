#include <std_include.hpp>

#include "database/models/mgo_data.hpp"
#include "database/models/mgo_characters.hpp"
#include "database/models/mgo_stats.hpp"

#include "cmd_mgo_mission_result.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_mgo_mission_result::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto& char_index_j = data["char_index"];
		const auto& earned_gp_j = data["earned_gp"];
		const auto& earned_xp_j = data["earned_xp"];
		const auto& gp_boost_mag_j = data["gp_boost_mag"];
		const auto& xp_boost_mag_j = data["xp_boost_mag"];
		const auto& rule_type_j = data["rule_type"];

		auto& actions_list = data["actions_list"];

		if (!char_index_j.is_number_unsigned() || !earned_gp_j.is_number_unsigned() || !earned_xp_j.is_number_unsigned() ||
			!gp_boost_mag_j.is_number_unsigned() || !xp_boost_mag_j.is_number_unsigned() || !actions_list.is_array() ||
			!rule_type_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto& stats_map = database::mgo_stats::get_stats_map();
		const auto rule_type = rule_type_j.get<std::uint32_t>();

		const auto rule_id = database::mgo_stats::get_rule_id(rule_type);

		/*for (auto i = 0ull; i < actions_list.size(); i++)
		{
			if (!actions_list[i].is_object())
			{
				continue;
			}

			const auto& id_j = actions_list[i]["key"];
			const auto& value_j = actions_list[i]["value"];

			if (!id_j.is_number_unsigned() || !value_j.is_number_unsigned())
			{
				continue;
			}

			const auto id = id_j.get<std::uint32_t>();
			const auto value = value_j.get<std::uint32_t>();
			const auto iter = stats_map.find(id);

			if (iter == stats_map.end())
			{
				continue;
			}

			if (iter->second.is_rule_specific)
			{
				database::mgo_stats::add_stat(player->get_id(), database::mgo_stats::rule_none, id, value);
			}
			else
			{
				if (rule_id != database::mgo_stats::rule_none)
				{
					database::mgo_stats::add_stat(player->get_id(), rule_id, id, value);
				}
			}
		}*/

		const auto char_index = char_index_j.get<std::uint32_t>();
		const auto earned_xp = earned_xp_j.get<std::uint32_t>();

		const auto earned_gp = earned_gp_j.get<std::uint32_t>();
		const auto gp_boost_mag = gp_boost_mag_j.get<std::uint32_t>();
		const auto xp_boost_mag = xp_boost_mag_j.get<std::uint32_t>();

		database::mgo_data::set_boost(player->get_id(), xp_boost_mag, gp_boost_mag);

		const auto current_gp = database::mgo_data::add_gp_coins(player->get_id(), earned_gp);
		const auto current_xp = database::mgo_characters::increase_xp(player->get_id(), char_index, earned_xp);

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());

		result["current_gp"] = current_gp;
		result["current_xp"] = current_xp;
		result["earned_gp"] = earned_gp;
		result["earned_xp"] = earned_xp;
		result["character_index"] = char_index;

		result["rank_param"]["current_rank_xp"] = mgo_data->get_rank_xp();
		result["rank_param"]["earned_rank_xp"] = 0;
		result["rank_param"]["rank_xp_list"] = nlohmann::json::array();

		for (auto i = 0; i < 16; i++)
		{
			result["rank_param"]["rank_xp_list"][i] = 0;
		}

		result["survival_params"]["current_survival_wins"] = 0;
		result["survival_params"]["earned_survival_gp"] = 0;
		result["survival_params"]["reward_category"] = "NotImplement";
		result["survival_params"]["reward_id_a"] = 0;
		result["survival_params"]["reward_id_b"] = 0;
		result["survival_params"]["reward_id_c"] = 0;
		result["survival_params"]["survival_update_key"] = 0;

		result["ucd"] = "";

		return result;
	}

	bool cmd_mgo_mission_result::needs_player()
	{
		return true;
	}
}
