#pragma once

#include "types/platform_handler.hpp"

namespace emulator
{
	class api_endpoint : public endpoint_handler
	{
	public:
		std::optional<std::string> handle_command(const utils::request_params& params, const std::string& data) override;
		virtual nlohmann::json handle_request(const utils::request_params& params, const std::string& data) = 0;
	};

	class api_handler final : public platform_handler
	{
	public:
		api_handler();

	private:

	};
}
