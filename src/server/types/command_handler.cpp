#include <std_include.hpp>

#include "command_handler.hpp"

namespace emulator
{
	nlohmann::json error(const std::string& id)
	{
		nlohmann::json result;
		result["result"] = id;
		return result;
	}

	nlohmann::json error(const std::uint32_t id)
	{
		nlohmann::json result;
		result["result"] = game::get_error(id);
		return result;
	}

	nlohmann::json resource(const std::uint32_t id)
	{
		auto resource = utils::resources::load_json(id);
		return resource;
	}

	nlohmann::json player_info(const std::uint64_t player_id, const std::uint64_t account_id)
	{
		nlohmann::json info;

		info["npid"]["handler"]["data"] = "";
		info["npid"]["handler"]["dummy"] = {0, 0, 0};
		info["npid"]["handler"]["term"] = 0;
		info["npid"]["opt"] = {0, 0, 0, 0, 0, 0, 0, 0};
		info["npid"]["reserved"] = {0, 0, 0, 0, 0, 0, 0, 0};
		info["player_id"] = player_id;
		info["player_name"] = account_id == 0 ? "NotImplement" : std::format("{}_player01", account_id);
		info["ugc"] = player_id != 0 ? 1 : 0;
		info["xuid"] = account_id;

		return info;
	}

	nlohmann::json player_info(const database::players::player& player)
	{
		return player_info(player.get_id(), player.get_account_id());
	}

	nlohmann::json player_info(const std::optional<database::players::player>& player)
	{
		return player_info(*player);
	}

	std::optional<database::players::player> get_target_player(const nlohmann::json& target, bool* has_id)
	{
		if (!target.is_object())
		{
			return {};
		}

		const auto get_id = [&](const std::string& key)
			-> std::optional<std::uint64_t>
		{
			if (!target.contains(key) || !target[key].is_number_unsigned())
			{
				return {};
			}

			const auto id = target[key].get<std::uint64_t>();
			if (id == 0u)
			{
				return {};
			}

			return {id};
		};

		const auto player_id_opt = get_id("player_id");
		const auto steam_id_opt = get_id("steam_id");

		std::optional<database::players::player> target_player;

		if (player_id_opt.has_value())
		{
			target_player = database::players::find(player_id_opt.value());
			if (has_id != nullptr)
			{
				*has_id = true;
			}
		}
		else if (steam_id_opt.has_value())
		{
			target_player = database::players::find_from_account(steam_id_opt.value());
			if (has_id != nullptr)
			{
				*has_id = true;
			}
		}

		return target_player;
	}

	void merge_json(nlohmann::json& data, const nlohmann::json& extra_data)
	{
		if (!extra_data.is_object())
		{
			return;
		}

		for (const auto& [k, v] : extra_data.items())
		{
			if (v.is_object())
			{
				if (data[k].is_object())
				{
					merge_json(data[k], v);
				}
				else
				{
					data[k] = v;
				}
			}
			else
			{
				data[k] = v;
			}
		}
	}
}
