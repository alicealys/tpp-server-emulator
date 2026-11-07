#include <std_include.hpp>

#include "cmd_get_fob_deploy_list.hpp"

#include "database/models/fobs.hpp"
#include "database/models/fob_deploy_assists.hpp"

namespace emulator::tpp
{
	nlohmann::json cmd_get_fob_deploy_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result;

		auto& cluster_index_j = data["cluster_index"];
		auto& motherbase_id_j = data["motherbase_id"];
		auto& security_level_j = data["security_level"];

		if (!cluster_index_j.is_number_unsigned() || !motherbase_id_j.is_number_unsigned() || !security_level_j.is_number_unsigned())
		{
			return error(ERR_INVALIDARG);
		}

		const auto cluster_index = cluster_index_j.get<std::uint8_t>();
		const auto motherbase_id = motherbase_id_j.get<std::uint32_t>();
		const auto security_level = security_level_j.get<std::uint8_t>();

		if (cluster_index > game::fob_clusters_count || security_level > 99)
		{
			return error(ERR_INVALIDARG);
		}

		const auto fob = database::fobs::get_fob(motherbase_id);
		if (!fob.has_value())
		{
			return error(ERR_DATABASE);
		}

		const auto& cluster_param = fob->get_cluster_param();
		const auto& cluster_param_single = cluster_param.param[game::cluster_index_map[cluster_index]];

		database::fob_deploy_assists::deploy_info_t info{};
		info.generate(cluster_param_single, security_level);
		info.to_json(result["deploy_param"]);

		return result;
	}

	bool cmd_get_fob_deploy_list::needs_player()
	{
		return true;
	}
}
