#include <std_include.hpp>

#include "cmd_purchase_fob.hpp"

#include "database/models/fobs.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_purchase_fob::execute(nlohmann::json& data, const std::string& session_key)
	{
		nlohmann::json result;

		result["result"] = "NOERR";

		const auto player = database::players::find_by_session_id(session_key);
		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}
		
		const auto& area_id_j = data["area_id"];
		
		const auto p_data = database::player_data::find(player->get_id());
		const auto fob_list = database::fobs::get_fob_list(player->get_id());

		if (!p_data.get() || fob_list.size() >= 4 || fob_list.size() < 1 || !area_id_j.is_number_integer())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto area_id = area_id_j.get<std::uint32_t>();
		const auto area_opt = database::fobs::get_area(area_id);
		if (!area_opt.has_value())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto& area = area_opt.value();
		const auto price = area["price"].get<std::uint32_t>();

		if (database::player_data::spend_coins(player->get_id(), price))
		{
			database::fobs::create(player->get_id(), area_id);
		}
		else
		{
			result["result"] = "ERR_MBCOIN_SHORTAGE";
		}
		
		return result;
	}
}
