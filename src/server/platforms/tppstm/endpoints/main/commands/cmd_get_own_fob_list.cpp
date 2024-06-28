#include <std_include.hpp>

#include "cmd_get_own_fob_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_get_own_fob_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["result"] = "NOERR";

		if (!player.has_value())
		{
			return error(ERR_INVALID_SESSION);
		}

		result["fob"] = nlohmann::json::array();
		result["enable_security_challenge"] = player->is_security_challenge_enabled();

		auto fob_list = database::fobs::get_fob_list(player->get_id());
		auto index = 0;
		for (auto& fob : fob_list)
		{
			auto area_opt = database::fobs::get_area(fob.get_area_id());
			auto& area = area_opt.value();
			result["fob"][index]["area_id"] = fob.get_area_id();
			result["fob"][index]["cluster_param"] = nlohmann::json::array();
			result["fob"][index]["construct_param"] = fob.get_construct_param();
			result["fob"][index]["fob_index"] = index;
			result["fob"][index]["mother_base_id"] = fob.get_id();
			result["fob"][index]["platform_count"] = 0;
			result["fob"][index]["price"] = area["price"];
			result["fob"][index]["security_rank"] = 0;

			++index;
		}
		
		return result;
	}
}
