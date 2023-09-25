#include <std_include.hpp>

#include "cmd_get_login_param.hpp"

#include "database/models/items.hpp"

#include <utils/nt.hpp>

namespace tpp
{
	cmd_get_login_param::cmd_get_login_param()
	{
		this->list_ = nlohmann::json::parse(utils::nt::load_resource(RESOURCE_LOGIN_PARAM));
	}

	nlohmann::json cmd_get_login_param::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result = this->list_;
		//result["server_product_params"] = database::items::get_static_list_json();
		return result;
	}
}
