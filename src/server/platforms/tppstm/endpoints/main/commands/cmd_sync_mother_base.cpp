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

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		auto& mother_base_param = data["mother_base_param"];
		if (mother_base_param.is_array())
		{
			std::vector<game::fob_param> fob_params;

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

				game::fob_param param{};

				param.platform_count = param_j["platform_count"].get<std::uint8_t>();
				param.security_rank = param_j["security_rank"].get<std::uint8_t>();
				param.construct_param.packed = param_j["construct_param"].get<std::uint32_t>();

				if (!database::fobs::parse_cluster_param(param_j["cluster_param"], param.cluster_param))
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
		
		nlohmann::json mb_data;
		mb_data["equip_flag"] = data["equip_flag"];
		mb_data["equip_grade"] = data["equip_grade"];
		mb_data["invalid_fob"] = data["invalid_fob"];
		mb_data["pf_skill_staff"] = data["pf_skill_staff"];
		mb_data["local_base_param"] = data["local_base_param"];
		mb_data["name_plate_id"] = data["name_plate_id"];
		mb_data["mother_base_num"] = data["mother_base_num"];
		mb_data["pickup_open"] = data["pickup_open"];
		mb_data["section_open"] = data["section_open"];
		mb_data["security_level"] = data["security_level"];
		mb_data["tape_flag"] = data["tape_flag"];

		database::player_data::sync_motherbase(player->get_id(), mb_data);
		
		result["version"] = 0;

		return result;
	}
}
