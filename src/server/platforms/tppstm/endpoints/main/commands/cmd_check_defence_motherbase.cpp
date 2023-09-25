#include <std_include.hpp>

#include "cmd_check_defence_motherbase.hpp"

#include "database/models/fobs.hpp"
#include "database/models/player_records.hpp"
#include "database/models/players.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	nlohmann::json cmd_check_defence_motherbase::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;
		result["result"] = "NOERR";

		if (!player.has_value())
		{
			result["result"] = "ERR_INVALID_SESSION";
			return result;
		}

		const auto& owner_player_id_j = data["owner_player_id"];
		if (!owner_player_id_j.is_number_unsigned())
		{
			result["result"] = "ERR_INVALIDARG";
			return result;
		}

		const auto owner_player_id = owner_player_id_j.get<std::uint64_t>();
		const auto active_sneak = database::players::find_active_sneak(owner_player_id, true);
		const auto active_sneak_defense = database::players::find_active_sneak(owner_player_id, false);

		if (!active_sneak.has_value())
		{
			result["check_result"] = 0;
			return result;
		}

		if (active_sneak.has_value() && active_sneak_defense.has_value())
		{
			result["check_result"] = 1;
			return result;
		}
		
		if (active_sneak.has_value() && !active_sneak_defense.has_value())
		{
			result["check_result"] = 2;
			return result;
		}

		return result;
	}
}
