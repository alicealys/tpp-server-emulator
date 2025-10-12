#include <std_include.hpp>

#include "database/models/mgo_characters.hpp"

#include "cmd_set_mgo_loadout.hpp"

namespace emulator::mgo
{
	bool cmd_set_mgo_loadout::set_loadout(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		const auto char_count = database::mgo_characters::get_character_count(player->get_id());
		auto& loadout = data["loadout"];

		if (!loadout.is_object())
		{
			return false;
		}

		auto& loadout_character_list = loadout["character_list"];
		if (!loadout_character_list.is_array())
		{
			return false;
		}

		const auto update_count = std::min(char_count, loadout_character_list.size());

		for (auto i = 0ull; i < update_count; i++)
		{
			if (!loadout_character_list[i].is_object() || !loadout_character_list[i]["loadout_list"].is_array())
			{
				return false;
			}
		}

		for (auto i = 0ull; i < update_count; i++)
		{
			auto& loadout_list = loadout_character_list[i]["loadout_list"];

			const auto loadout_count = static_cast<std::uint32_t>(std::min(static_cast<std::uint64_t>(loadout_list.size()), 
				database::mgo_characters::max_loadout_count));

			auto loadouts = std::make_shared<database::mgo_characters::character_loadouts_t>();

			auto bad_loadout = false;
			for (auto o = 0u; o < loadout_count; o++)
			{
				if (!database::mgo_characters::character_loadout_t::parse(loadout_list[o], loadouts->loadouts[o]))
				{
					bad_loadout = true;
					break;
				}
			}

			if (bad_loadout)
			{
				continue;
			}

			database::mgo_characters::update_character_loadouts(player->get_id(), static_cast<std::uint32_t>(i), loadouts, loadout_count);
		}

		return true;
	}

	nlohmann::json cmd_set_mgo_loadout::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		if (!this->set_loadout(data, player))
		{
			return error(ERR_INVALIDARG);
		}

		return {};
	}

	bool cmd_set_mgo_loadout::needs_player()
	{
		return true;
	}
}
