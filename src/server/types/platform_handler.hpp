#pragma once

#include "base_handler.hpp"
#include "endpoint_handler.hpp"

namespace tpp
{
	class platform_handler : public base_handler<endpoint_handler>
	{
	public:
		std::optional<std::string> handle_endpoint(const utils::request_params& params, const std::string& endpoint, const std::string& body);

		void print_handler_name([[ maybe_unused ]] const std::string& name) override;
	};
}
