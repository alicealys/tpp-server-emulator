#include <std_include.hpp>

#include "cmd_get_informationlist2.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_informationlist2::cmd_get_informationlist2()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_INFORMATIONLIST2));
	}

	nlohmann::json cmd_get_informationlist2::execute(nlohmann::json& data, const std::string& session_key)
	{
		return this->list_;
	}
}
