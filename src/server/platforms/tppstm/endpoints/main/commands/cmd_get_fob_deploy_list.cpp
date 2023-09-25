#include <std_include.hpp>

#include "cmd_get_fob_deploy_list.hpp"

#include "database/models/fobs.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_fob_deploy_list::cmd_get_fob_deploy_list()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_FOB_DEPLOY_LIST));
	}

	nlohmann::json cmd_get_fob_deploy_list::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
