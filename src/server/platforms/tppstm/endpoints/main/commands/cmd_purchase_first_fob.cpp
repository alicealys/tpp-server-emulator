#include <std_include.hpp>

#include "cmd_purchase_first_fob.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_purchase_first_fob::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		result["result"] = "NOERR";

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		const auto& area_id_j = data["area_id"];
		
		const auto fob_list = database::fobs::get_fob_list(player->get_id());
		if (fob_list.size() != 0 || !area_id_j.is_number_integer())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto area_id = area_id_j.get<std::uint32_t>();
		const auto area = database::fobs::get_area(area_id);
		if (!area.has_value())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		database::fobs::create(player->get_id(), area_id);
		
		return result;
	}
}
