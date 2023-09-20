#include <std_include.hpp>

#include "cmd_gdpr_check.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_gdpr_check::cmd_gdpr_check()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_GDPR_CHECK));
	}

	nlohmann::json cmd_gdpr_check::execute(nlohmann::json& data, const std::string& session_key)
	{
		return this->list_;
	}
}
