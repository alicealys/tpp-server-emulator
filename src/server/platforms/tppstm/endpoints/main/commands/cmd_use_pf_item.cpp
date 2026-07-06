#include <std_include.hpp>

#include "cmd_use_pf_item.hpp"

#include "database/models/pf_league.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_use_pf_item::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& defence_item_j = data["defence_item"];
		auto& attack_item_j = data["attack_item"];
		auto& section_j = data["section"];

		if (!defence_item_j.is_number_unsigned() || !attack_item_j.is_number_unsigned() || !section_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto defence_item = std::min(1u, defence_item_j.get<std::uint32_t>());
		const auto attack_item = std::min(1u, attack_item_j.get<std::uint32_t>());
		const auto section = section_j.get<std::uint32_t>();

		const auto league = database::pf_league::get_current_pf_league();
		if (!league.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto self = database::pf_league::get_player_competitor_instance(league->get_id(), player->get_id());
		if (!self.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto battles = database::pf_league::get_player_battles(self->get_bracket_id(), self->get_player_id());
		auto found = false;
		for (const auto& battle : battles)
		{
			if (battle.get_section() != section)
			{
				continue;
			}

			found = true;

			if (battle.get_defender_id() == player->get_id())
			{
				if (battle.get_defender_buff() == 0 && defence_item >= 1)
				{
					if (database::player_data::use_league_item(player->get_id(), 0u, 1u, true))
					{
						database::pf_league::inc_battle_buff(battle.get_id(), 0u, 1u, true);
					}
				}
				else if (battle.get_defender_buff() >= 1 && defence_item == 0)
				{
					if (database::player_data::use_league_item(player->get_id(), 0u, 1u, false))
					{
						database::pf_league::inc_battle_buff(battle.get_id(), 0u, 1u, false);
					}
				}
			}
			else if (battle.get_attacker_id() == player->get_id())
			{
				if (battle.get_attacker_buff() == 0 && attack_item >= 1)
				{
					if (database::player_data::use_league_item(player->get_id(), 1u, 0u, true))
					{
						database::pf_league::inc_battle_buff(battle.get_id(), 1u, 0u, true);
					}
				}
				else if (battle.get_attacker_buff() >= 1 && attack_item == 0)
				{
					if (database::player_data::use_league_item(player->get_id(), 1u, 0u, false))
					{
						database::pf_league::inc_battle_buff(battle.get_id(), 1u, 0u, false);
					}
				}
			}
		}

		if (!found)
		{
			return error(ERR_INVALIDARG);
		}

		return result;
	}

	bool cmd_use_pf_item::needs_player()
	{
		return true;
	}
}
