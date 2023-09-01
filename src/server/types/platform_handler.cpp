#include <std_include.hpp>

#include "platform_handler.hpp"

namespace tpp
{
	std::optional<std::string> platform_handler::handle_endpoint(const std::string& endpoint, const std::string& data)
	{
		const auto handler = this->handlers_.find(endpoint);
		if (handler == this->handlers_.end())
		{
			return {};
		}

		return handler->second->handle_command(data);
	}
}
