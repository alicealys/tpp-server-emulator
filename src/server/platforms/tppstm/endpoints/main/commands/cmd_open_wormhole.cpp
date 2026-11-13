#include <std_include.hpp>

#include "cmd_open_wormhole.hpp"

#include "database/models/players.hpp"
#include "database/models/wormholes.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_open_wormhole::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& player_id_j = data["player_id"];
		auto& to_player_id_j = data["to_player_id"];
		auto& retaliate_score_j = data["retaliate_score"];
		auto& flag_j = data["flag"];
		auto& is_open_j = data["is_open"];

		if (!player_id_j.is_number_unsigned() || !to_player_id_j.is_number_unsigned() || 
			!retaliate_score_j.is_number_unsigned() || !flag_j.is_string() || !is_open_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto active_sneak = database::players::find_active_sneak_from_player(player->get_id());

		const auto from_player_id = player_id_j.get<std::uint64_t>();  // fob owner
		auto to_player_id = to_player_id_j.get<std::uint64_t>(); // attacker
		const auto is_open = is_open_j.get<std::uint8_t>() == 1;

		const auto owner = database::players::find(from_player_id);
		if (!owner.has_value())
		{
			return error(ERR_PLAYER_NOTFOUND);
		}
		
		const auto retaliate_score = retaliate_score_j.get<std::uint32_t>();
		const auto flag = flag_j.get<std::string>();
		const auto flag_id = database::wormholes::get_flag_id(flag);

		if (flag_id == database::wormholes::wormhole_flag_invalid)
		{
			return error(ERR_INVALIDARG);
		}

		if (to_player_id != player->get_id())
		{
			return error(ERR_DATABASE);
		}

		std::uint64_t follower_id = 0u;
		if (active_sneak.has_value() && active_sneak->get_wormhole_player_id() != 0u)
		{
			follower_id = player->get_id();
			to_player_id = active_sneak->get_wormhole_player_id();
		}

		if (!owner->is_security_challenge_enabled())
		{
			const auto capped_score = std::min(database::wormholes::max_retaliate_score, retaliate_score);
			database::wormholes::add_wormhole(from_player_id, to_player_id, follower_id, flag_id, is_open, capped_score);
			const auto status = database::wormholes::get_wormhole_status(from_player_id, to_player_id);

			result["is_new_open"] = std::uint8_t(status.open && status.first);
		}
		else
		{
			result["is_new_open"] = 0;
		}

		result["player_id"] = from_player_id;
		result["to_player_id"] = to_player_id;

		return result;
	}

	bool cmd_open_wormhole::needs_player()
	{
		return true;
	}
}
