#include <std_include.hpp>

#include "cmd_get_fob_param.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_fob_param::cmd_get_fob_param()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_FOB_PARAM));
	}

	nlohmann::json cmd_get_fob_param::execute(nlohmann::json& data, const std::string& session_key)
	{
		return this->list_;
	}
}
