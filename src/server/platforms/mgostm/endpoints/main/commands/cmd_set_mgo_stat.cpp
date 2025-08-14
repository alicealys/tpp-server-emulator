#include <std_include.hpp>

#include "database/models/mgo_stat.hpp"

#include "cmd_set_mgo_stat.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_set_mgo_stat::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& stat = data["stat"];
		if (!stat.is_object() || !stat["stat_list"].is_array())
		{
			return error(ERR_INVALIDARG);
		}

		auto& list = stat["stat_list"];
		const auto id_set = database::mgo_stat::get_id_set();

		for (auto i = 0ull; i < list.size(); i++)
		{
			const auto& id_j = list[i]["id"];
			const auto& value_j = list[i]["value"];

			if (!id_j.is_number_unsigned() || !value_j.is_number_unsigned())
			{
				continue;
			}

			const auto id = id_j.get<std::uint32_t>();
			const auto value = value_j.get<std::uint32_t>();
			if (!id_set.contains(id))
			{
				continue;
			}

			database::mgo_stat::set_stat(player->get_id(), id, value);
		}

		return result;
	}

	bool cmd_set_mgo_stat::needs_player()
	{
		return true;
	}
}
