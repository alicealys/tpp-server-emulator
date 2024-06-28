#include <std_include.hpp>

#include "platform_handler.hpp"

#include "component/console.hpp"

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
			console::error("Error handling command: %s\n", e.what());
			return {};
		}
	}

	void platform_handler::print_handler_name([[ maybe_unused ]] const std::string& name)
	{
		console::log("Registering endpoint \"%s\"\n", name.data());
	}
}
