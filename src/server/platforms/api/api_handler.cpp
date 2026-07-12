#include <std_include.hpp>

#include "endpoints/steam_openid.hpp"

#include "api_handler.hpp"

namespace emulator
{
	std::optional<std::string> api_endpoint::handle_command(const utils::request_params& params, const std::string& data)
	{
		const auto result = this->handle_request(params, data);
		return result.dump();
	}

	api_handler::api_handler()
	{
		this->set_content_type("application/json");
		this->register_handler<steam_openid>("steam_openid");
	}
}
