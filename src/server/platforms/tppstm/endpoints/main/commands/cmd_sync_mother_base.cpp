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
			std::vector<database::fobs::fob> fobs;
			for (auto i = 0ull; i < mother_base_param.size(); i++)
			{
				auto& param = mother_base_param[i];
				if (!param["construct_param"].is_number_integer() ||
					!param["platform_count"].is_number_integer() ||
					!param["security_rank"].is_number_integer() ||
					!param["cluster_param"].is_array())
				{
					return error(ERR_INVALIDARG);
				}

				const auto cluster_security = param["cluster_param"][0]["cluster_security"].get<std::uint32_t>();
				printf("cluster security %i\n", cluster_security);


				const auto construct_param = param["construct_param"].get<std::uint32_t>();
				printf("construct_param %i\n", construct_param);

				fobs.emplace_back(param);
			}

			if (fobs.size() > 0)
			{
				database::fobs::sync_data(player->get_id(), fobs);
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
