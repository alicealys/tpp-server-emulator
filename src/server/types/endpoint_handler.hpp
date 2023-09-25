#pragma once

#include "base_handler.hpp"
#include "command_handler.hpp"
#include "utils/http_server.hpp"

namespace tpp
{
	class endpoint_handler : public base_handler<command_handler>
	{
	public:
		virtual std::optional<nlohmann::json> decrypt_request(const std::string& data, std::optional<database::players::player>& player)
		{
			return {};
		}

		virtual bool verify_request(const nlohmann::json& request)
		{
			return false;
		}

		virtual std::optional<std::string> encrypt_response(const nlohmann::json& request, nlohmann::json data, const std::optional<database::players::player>& player)
		{
			return {};
		}

		virtual std::optional<std::string> handle_command(const utils::request_params& params, const std::string& data);
	};
}
