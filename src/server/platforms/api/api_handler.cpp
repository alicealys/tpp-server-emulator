#include <std_include.hpp>

#include "endpoints/steam_openid.hpp"
#include "endpoints/status.hpp"
#include "endpoints/delete_account.hpp"
#include "utils/config.hpp"

#include "api_handler.hpp"

namespace emulator
{
	std::optional<std::string> api_endpoint::handle_command(const utils::request_params& params)
	{
		const auto result = this->handle_request(params);
		return result.dump();
	}

	api_handler::api_handler()
	{
		const auto enable = config::get<bool>("enable_web_api");
		if (!enable)
		{
			return;
		}

		this->set_content_type("application/json");
		this->register_handler<steam_openid>("steam_openid");
		this->register_handler<status>("status");
		this->register_handler<delete_account>("delete_account");
	}
}
