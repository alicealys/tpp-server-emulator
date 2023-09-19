#include <std_include.hpp>

#include "cmd_sync_mother_base.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"
#include "database/models/player_data.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_sync_mother_base::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		auto& mother_base_param = data["mother_base_param"];
		if (mother_base_param.is_array())
		{
			std::vector<database::fobs::fob> fobs;
			for (auto i = 0; i < mother_base_param.size(); i++)
			{
				auto& param = mother_base_param[i];
				if (!param["construct_param"].is_number_integer() ||
					!param["platform_count"].is_number_integer() ||
					!param["security_rank"].is_number_integer() ||
					!param["cluster_param"].is_array())
				{
					result["result"] = "ERR_INVALIDARG";
					return result;
				}

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

		return result;
	}
}
