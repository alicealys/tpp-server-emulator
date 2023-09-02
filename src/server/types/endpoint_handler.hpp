#pragma once

#include "base_handler.hpp"
#include "command_handler.hpp"

namespace tpp
{
	class endpoint_handler : public base_handler<command_handler>
	{
	public:
		virtual std::optional<nlohmann::json> decrypt_request(const std::string& data)
		{
			return {};
		}

		virtual bool verify_request(const nlohmann::json& request)
		{
			return false;
		}

		virtual std::optional<std::string> encrypt_response(const nlohmann::json& request, nlohmann::json data)
		{
			return {};
		}

		std::optional<std::string> handle_command(const std::string& data);
	};
}
