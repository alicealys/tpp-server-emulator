#include <std_include.hpp>

#include "cmd_get_login_param.hpp"

#include "database/models/items.hpp"
#include "database/models/fob_events.hpp"

namespace emulator::tpp
{
	cmd_get_login_param::cmd_get_login_param()
	{
		this->list_ = resource(RESOURCE_LOGIN_PARAM);
	}

	nlohmann::json cmd_get_login_param::execute(nlohmann::json& data, const std::optional<database::players::player>& player)
	{
		nlohmann::json result = this->list_;

		result["server_product_params"] = database::items::get_static_list_json();

		const auto current_event = database::fob_events::get_current_event();
		if (current_event.has_value())
		{
			result["fob_event_task_list"]["one_event_task"] = current_event->one_event_task;
		}

		return result;
	}
}
