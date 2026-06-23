#include <std_include.hpp>

#include "cmd_create_player.hpp"

#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_records.hpp"
#include "database/models/player_data.hpp"
#include "database/models/event_rankings.hpp"
#include "database/models/mgo_data.hpp"
#include "database/models/mgo_characters.hpp"
#include "database/models/fobs.hpp"

namespace emulator::tpp
{
	namespace
	{
		database::player_data::resource_arrays_t* load_maxed_out_resources()
		{
			static database::player_data::resource_arrays_t resource_arrays{};
			for (auto i = 0; i < game::resource_array_types_t::count; i++)
			{
				for (auto o = 0; o < game::resource_type_count; o++)
				{
					if (o == game::nuclear)
					{
						continue;
					}

					resource_arrays[i][o] = game::resource_caps[i][o];
				}
			}
			return &resource_arrays;
		}

		database::player_data::resource_arrays_t* get_maxed_out_resources()
		{
			static auto resources = load_maxed_out_resources();
			return resources;
		}
	}

	nlohmann::json cmd_create_player::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["xuid"] = {};

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		database::player_records::find_or_create(player->get_id());
		database::player_data::find_or_create(player->get_id());
		database::event_rankings::create_entries(player->get_id());

		if (database::vars.signup_bonus)
		{
			const auto fobs = database::fobs::get_fob_list(player->get_id());
			if (fobs.empty())
			{
				database::fobs::create(player->get_id(), 0);
				database::fobs::create(player->get_id(), 0);
				database::fobs::create(player->get_id(), 0);
				database::fobs::create(player->get_id(), 0);

				const auto maxed_resources = get_maxed_out_resources();
				database::player_data::set_resources_as_sync(player->get_id(), *maxed_resources, database::vars.max_local_gmp, database::vars.max_server_gmp);
			}
		}

		const auto mgo_data = database::mgo_data::find(player->get_id());
		if (!mgo_data.has_value())
		{
			database::mgo_data::create(player->get_id());

			database::mgo_characters::character_progression_params params{};
			params.legendary = 1;
			params.prestige = database::mgo_characters::max_prestige;
			params.xp = 10000000u;
			for (auto i = 0u; i < database::mgo_characters::total_character_count; i++)
			{
				if (database::vars.signup_bonus && database::mgo_characters::create_character(player->get_id(), static_cast<std::uint32_t>(i)))
				{
					database::mgo_characters::update_character_progression(player->get_id(), i, params);
				}
			}

			if (database::vars.signup_bonus)
			{
				database::player_data::add_mb_coins(player->get_id(), 100000000);
				database::mgo_data::add_gp_coins(player->get_id(), 100000000);
			}
		}

		result["player_id"] = player->get_id();
		result["result"] = "NOERR";

		return result;
	}
}
