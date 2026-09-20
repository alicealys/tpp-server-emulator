#include <std_include.hpp>

#include "cmd_mining_resource.hpp"

#include "database/models/items.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_mining_resource::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		const auto player_data = database::player_data::find(player->get_id());
		if (!player_data.has_value())
		{
			return error(ERR_DATABASE);
		}

		database::player_data::resource_arrays_t resource_arrays{};
		player_data->get_resource_arrays(resource_arrays);

		const auto do_resource = [](nlohmann::json& data, const std::uint32_t biotic, const std::uint32_t common, 
			const std::uint32_t fuel, const std::uint32_t minor, const std::uint32_t precious)
		{
			data["biotic_resource"] = biotic;
			data["common_metal"] = common;
			data["fuel_resource"] = fuel;
			data["minor_metal"] = minor;
			data["precious_metal"] = precious;
		};

		result["break_time"] = 2100;

		auto& process = resource_arrays[game::unprocessed_server];
		auto& usable = resource_arrays[game::processed_server];

		// do mining?

		do_resource(result["after_mining_resource_processing"],
			process[game::BIOTIC_RESOURCE],
			process[game::COMMON_METAL],
			process[game::FUEL_RESOURCE],
			process[game::MINOR_METAL],
			process[game::PRECIOUS_METAL]);

		do_resource(result["after_mining_resource_usable"],
			usable[game::BIOTIC_RESOURCE],
			usable[game::COMMON_METAL],
			usable[game::FUEL_RESOURCE],
			usable[game::MINOR_METAL],
			usable[game::PRECIOUS_METAL]);

		do_resource(result["before_process_resource_processing"],
			process[game::BIOTIC_RESOURCE],
			process[game::COMMON_METAL],
			process[game::FUEL_RESOURCE],
			process[game::MINOR_METAL],
			process[game::PRECIOUS_METAL]);

		do_resource(result["before_process_resource_usable"],
			usable[game::BIOTIC_RESOURCE],
			usable[game::COMMON_METAL],
			usable[game::FUEL_RESOURCE],
			usable[game::MINOR_METAL],
			usable[game::PRECIOUS_METAL]);

		// do processing ?

		do_resource(result["after_process_resource_processing"],
			process[game::BIOTIC_RESOURCE],
			process[game::COMMON_METAL],
			process[game::FUEL_RESOURCE],
			process[game::MINOR_METAL],
			process[game::PRECIOUS_METAL]);

		do_resource(result["after_process_resource_usable"],
			usable[game::BIOTIC_RESOURCE],
			usable[game::COMMON_METAL],
			usable[game::FUEL_RESOURCE],
			usable[game::MINOR_METAL],
			usable[game::PRECIOUS_METAL]);

		do_resource(result["after_mining_resource"], 0u, 0u, 0u, 0u, 0u);
		do_resource(result["after_process_resource"], 0u, 0u, 0u, 0u, 0u);
		do_resource(result["before_mining_resource"], 0u, 0u, 0u, 0u, 0u);
		do_resource(result["before_process_resource"], 0u, 0u, 0u, 0u, 0u);
		do_resource(result["local_process_resource"], 0u, 0u, 0u, 0u, 0u);

		database::player_data::set_resources(player->get_id(), resource_arrays);

		return result;
	}

	bool cmd_mining_resource::needs_player()
	{
		return true;
	}
}
