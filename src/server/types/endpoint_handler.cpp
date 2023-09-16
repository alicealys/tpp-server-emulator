#include <std_include.hpp>

#include "endpoint_handler.hpp"

namespace tpp
{
	std::optional<std::string> endpoint_handler::handle_command([[maybe_unused]] const utils::request_params& params, const std::string& data)
	{
		const auto json_req_opt = this->decrypt_request(data);
		if (!json_req_opt.has_value())
		{
			return {};
		}

		const auto& json_req = json_req_opt.value();
		if (!this->verify_request(json_req))
		{
			return {};
		}

		const auto& session_key = json_req["session_key"];
		if (!session_key.is_string())
		{
			return {};
		}

		const auto msgid_str = json_req["data"]["msgid"].get<std::string>();

		const auto handler = this->handlers_.find(msgid_str);
		if (handler == this->handlers_.end())
		{
			return {};
		}

		printf("[Endpoint] Handling command \"%s\"\n", msgid_str.data());

		const auto json_res = handler->second->execute(json_req["data"], session_key.get<std::string>());
		return this->encrypt_response(json_req, json_res);
	}
}
