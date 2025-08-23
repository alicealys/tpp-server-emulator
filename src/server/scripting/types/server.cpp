#include <std_include.hpp>

#include "../engine.hpp"
#include "server.hpp"

namespace emulator::scripting
{
	void engine::setup_server()
	{
		this->state_["server"] = sol::state::create_table(this->state_.lua_state());

		this->state_["server"]["resource"] = sol::state::create_table(this->state_.lua_state());
		this->state_["server"]["resource"]["GDPR_CHECK"] = RESOURCE_GDPR_CHECK;
		this->state_["server"]["resource"]["CHALLENGE_TASK_REWARDS"] = RESOURCE_CHALLENGE_TASK_REWARDS;
		this->state_["server"]["resource"]["LOGIN_PARAM"] = RESOURCE_LOGIN_PARAM;
		this->state_["server"]["resource"]["TPP_INFORMATIONLIST2"] = RESOURCE_TPP_INFORMATIONLIST2;
		this->state_["server"]["resource"]["ITEM_LIST"] = RESOURCE_ITEM_LIST;
		this->state_["server"]["resource"]["MINING_RESOURCE"] = RESOURCE_MINING_RESOURCE;
		this->state_["server"]["resource"]["AREA_LIST"] = RESOURCE_AREA_LIST;
		this->state_["server"]["resource"]["FOB_PARAM"] = RESOURCE_FOB_PARAM;
		this->state_["server"]["resource"]["SECURITY_SETTINGS_PARAM"] = RESOURCE_SECURITY_SETTINGS_PARAM;
		this->state_["server"]["resource"]["FOB_DEPLOY_LIST"] = RESOURCE_FOB_DEPLOY_LIST;
		this->state_["server"]["resource"]["STEAM_SHOP_ITEM_LIST"] = RESOURCE_STEAM_SHOP_ITEM_LIST;
		this->state_["server"]["resource"]["MGO_PARAMETERS"] = RESOURCE_MGO_PARAMETERS;
		this->state_["server"]["resource"]["MGO_TITLE_LIST"] = RESOURCE_MGO_TITLE_LIST;
		this->state_["server"]["resource"]["MGO_INFORMATIONLIST2"] = RESOURCE_MGO_INFORMATIONLIST2;

		this->state_["server"]["resource"]["SQL_MYSQL"] = RESOURCE_SQL_MYSQL;
		this->state_["server"]["resource"]["SQL_SQLITE3"] = RESOURCE_SQL_SQLITE3;

		this->state_["server"]["error"] = sol::state::create_table(this->state_.lua_state());

		const auto& error_map = game::get_error_map();
		for (const auto& [id, error] : error_map)
		{
			this->state_["server"]["error"][error] = id;
		}

		this->state_["server"]["getresource"] = utils::resources::load;
		this->state_["server"]["geterror"] = game::get_error;

		this->state_["server"]["registercommand"] = [&](const std::string& name, const sol::protected_function& handler)
		{
			console::log("Registering script command \"%s\"", name.data());
			this->command_handlers_.insert(std::make_pair(name, handler));
		};

		this->state_["server"]["originalhandler"] = [&]
			-> nlohmann::json
		{
			if (!this->original_handler_.has_value())
			{
				return {};
			}

			return this->original_handler_->operator()();
		};
	}
}
