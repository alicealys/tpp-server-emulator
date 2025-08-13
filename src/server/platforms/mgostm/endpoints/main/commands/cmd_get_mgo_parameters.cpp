#include <std_include.hpp>

#include "cmd_get_mgo_parameters.hpp"

namespace emulator::mgo
{
	cmd_get_mgo_parameters::cmd_get_mgo_parameters()
	{
		this->list_ = utils::resources::load_json(RESOURCE_MGO_PARAMETERS);
	}

	nlohmann::json cmd_get_mgo_parameters::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
