#pragma once

#include "server.hpp"

#include <utils/cryptography.hpp>

namespace tpp
{
	class gate_handler : public endpoint_handler
	{
	public:
		gate_handler();

		nlohmann::json decrypt_request(const std::string& data) override;
		std::string encrypt_response(const std::uint32_t rq_id, const std::string& msgid, nlohmann::json data) override;

	private:
		utils::cryptography::blowfish blow_;

	};
}