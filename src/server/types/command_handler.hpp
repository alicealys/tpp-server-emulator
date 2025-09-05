#pragma once

#include "database/models/players.hpp"
#include "game/game.hpp"

namespace emulator
{
	class command_handler
	{
	public:
		virtual ~command_handler()
		{
		}

		virtual nlohmann::json execute(nlohmann::json& data, const std::optional<database::players::player>& player)
		{
			throw std::runtime_error("unimplemented command");
		};

		virtual bool needs_player()
		{
			return false;
		}

		virtual bool needs_ip_address()
		{
			return false;
		}
	};

	nlohmann::json error(const std::string& id);
	nlohmann::json error(const std::uint32_t id);
	nlohmann::json resource(const std::uint32_t id);
	nlohmann::json player_info(const std::uint64_t player_id, const std::uint64_t account_id);
	nlohmann::json player_info(const database::players::player& player);
	nlohmann::json player_info(const std::optional<database::players::player>& player);
	std::optional<database::players::player> get_target_player(const nlohmann::json& target);
	void merge_json(nlohmann::json& data, const nlohmann::json& extra_data);
}
