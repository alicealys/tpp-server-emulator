#include <std_include.hpp>

#include "platform_handler.hpp"

namespace tpp
{
	std::optional<std::string> platform_handler::handle_endpoint(const utils::request_params& params, const std::string& endpoint, const std::string& data)
	{
		const auto handler = this->handlers_.find(endpoint);
		if (handler == this->handlers_.end())
		{
			return {};
		}

		try
		{
			return handler->second->handle_command(params, data);
		}
		catch (const std::exception& e)
		{
			printf("error handling command: %s\n", e.what());
			return {};
		}
	}
}
