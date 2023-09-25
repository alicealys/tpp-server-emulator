#include <std_include.hpp>

#include "cmd_get_security_setting_param.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_security_setting_param::cmd_get_security_setting_param()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_SECURITY_SETTINGS_PARAM));
	}

	nlohmann::json cmd_get_security_setting_param::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		return this->list_;
	}
}
