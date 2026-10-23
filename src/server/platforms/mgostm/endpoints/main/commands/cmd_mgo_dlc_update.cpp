#include <std_include.hpp>

#include "database/models/mgo_data.hpp"

#include "cmd_mgo_dlc_update.hpp"

namespace emulator::mgo
{
	nlohmann::json cmd_mgo_dlc_update::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& dlc_flags_j = data["dlc_flags"];
		if (!dlc_flags_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto mgo_data = database::mgo_data::find_or_create(player->get_id());
		const auto new_dlc_flags = dlc_flags_j.get<std::uint32_t>() | mgo_data->get_dlc_flags();

		database::mgo_data::set_dlc_flags(player->get_id(), new_dlc_flags);

		result["now_dlc_flags"] = new_dlc_flags;
		result["old_dlc_flags"] = mgo_data->get_dlc_flags();

		return result;
	}

	bool cmd_mgo_dlc_update::needs_player()
	{
		return true;
	}
}
