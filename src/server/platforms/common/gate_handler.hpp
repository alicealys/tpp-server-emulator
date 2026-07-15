#pragma once

#include "types/endpoint_handler.hpp"

#include <utils/cryptography.hpp>

namespace emulator
{
	class gate_handler : public endpoint_handler
	{
	public:
		gate_handler();

		std::optional<nlohmann::json> decrypt_request(const std::string& data, std::optional<database::players::player>& player) override;
		bool verify_request(nlohmann::json& request) override;
		std::optional<std::string> encrypt_response(nlohmann::json& request, nlohmann::json& data,
			const std::optional<database::players::player>& player) override;

	private:
		utils::cryptography::blowfish blow_;

	};
}