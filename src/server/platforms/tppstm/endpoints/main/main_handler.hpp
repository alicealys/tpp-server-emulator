#pragma once

#include "types/endpoint_handler.hpp"

#include <utils/cryptography.hpp>

namespace tpp
{
	class main_handler : public endpoint_handler
	{
	public:
		main_handler();

		std::optional<nlohmann::json> decrypt_request(const std::string& data) override;
		bool verify_request(const nlohmann::json& request) override;
		std::optional<std::string> encrypt_response(const nlohmann::json& request, nlohmann::json data) override;

	private:
		utils::cryptography::blowfish blow_;

	};

	nlohmann::json error(const std::string& id);
}