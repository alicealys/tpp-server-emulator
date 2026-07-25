#include <std_include.hpp>

#include "cmd_sync_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_sync_mother_base::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& mother_base_param = data["mother_base_param"];
		if (mother_base_param.is_array())
		{
			std::vector<game::fob_param_t> fob_params;

			for (auto i = 0ull; i < mother_base_param.size(); i++)
			{
				auto& param_j = mother_base_param[i];
				if (!param_j["construct_param"].is_number_integer() ||
					!param_j["platform_count"].is_number_integer() ||
					!param_j["security_rank"].is_number_integer() ||
					!param_j["cluster_param"].is_array())
				{
					return error(ERR_INVALIDARG);
				}

				game::fob_param_t param{};
				if (!game::parse_fob_param(param_j, param))
				{
					return error(ERR_INVALIDARG);
				}

				fob_params.emplace_back(param);
			}

			if (fob_params.size() > 0)
			{
				database::fobs::sync_data(player->get_id(), fob_params);
			}
		}
		
		game::motherbase_t motherbase{};
		game::parse_motherbase(data, motherbase);
		database::player_data::sync_motherbase(player->get_id(), motherbase);
		
		result["version"] = 0;

		return result;
	}
	
	bool cmd_sync_mother_base::needs_player()
	{
		return true;
	}
}
