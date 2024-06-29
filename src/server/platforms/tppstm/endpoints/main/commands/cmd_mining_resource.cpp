#include <std_include.hpp>

#include "cmd_mining_resource.hpp"

#include "database/models/items.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_mining_resource::cmd_mining_resource()
	{
		this->list_ = resource(RESOURCE_MINING_RESOURCE);
	}

	nlohmann::json cmd_mining_resource::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
