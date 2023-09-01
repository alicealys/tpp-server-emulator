#include <std_include.hpp>

#include "endpoint_handler.hpp"

namespace tpp
{
	std::optional<std::string> endpoint_handler::handle_command(const std::string& data)
	{
		const auto json_req = this->decrypt_request(data);

		if (!this->verify_request(json_req))
		{
			return {};
		}

		const auto msgid_str = json_req["data"]["msgid"].get<std::string>();

		printf("handling command %s\n", msgid_str.data());

		const auto handler = this->handlers_.find(msgid_str);
		if (handler == this->handlers_.end())
		{
			return {};
		}

		const auto json_res = handler->second->execute(json_req["data"]);
		const auto response = this->encrypt_response(json_req, json_res);

		return {response};
	}
}
