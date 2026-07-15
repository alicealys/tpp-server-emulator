#pragma once

#include "base_handler.hpp"
#include "endpoint_handler.hpp"

namespace emulator
{
	class platform_handler : public base_handler<endpoint_handler>
	{
	public:
		platform_handler();
		std::optional<std::string> handle_endpoint(const utils::request_params& params, const std::string& endpoint);

		void print_handler_name([[ maybe_unused ]] const std::string& name) override;
		void set_content_type(const std::string& content_type);
		std::string get_content_type();

	private:
		std::string content_type_;

	};
}
