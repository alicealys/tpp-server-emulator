#include <std_include.hpp>

#include "platform_handler.hpp"

#include "component/console.hpp"

namespace emulator
{
	platform_handler::platform_handler()
	{
		this->content_type_ = "text/plain";
	}

	std::optional<std::string> platform_handler::handle_endpoint(const utils::request_params& params, const std::string& endpoint)
	{
		const auto handler = this->handlers_.find(endpoint);
		if (handler == this->handlers_.end())
		{
			return {};
		}

		try
		{
			return handler->second->handle_command(params);
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

	void platform_handler::set_content_type(const std::string& content_type)
	{
		this->content_type_ = content_type;
	}

	std::string platform_handler::get_content_type()
	{
		return this->content_type_;
	}
}
