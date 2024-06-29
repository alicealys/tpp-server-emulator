#include <std_include.hpp>

#include "endpoint_handler.hpp"

#include "component/console.hpp"

namespace tpp
{
	std::optional<std::string> endpoint_handler::handle_command([[maybe_unused]] const utils::request_params& params, 
		const std::string& data)
	{
		std::optional<database::players::player> player;
		auto json_req_opt = this->decrypt_request(data, player);
		if (!json_req_opt.has_value())
		{
			return {};
		}

		auto& json_req = json_req_opt.value();
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

#ifdef DEBUG
		static std::atomic_int64_t exec_id = 0;
		auto id = exec_id++;
		const auto start = std::chrono::high_resolution_clock::now();
		const auto _0 = gsl::finally([=]
		{
			const auto end = std::chrono::high_resolution_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			console::debug("[Endpoint] Took %lli msec (%lli)", diff, id);
		});

		console::debug("[Endpoint] Handling command \"%s\" (%lli)\n", msgid_str.data(), id);
#endif

		nlohmann::json json_res;
		if (handler->second->needs_player() && !player.has_value())
		{
			json_res = error(ERR_INVALID_SESSION);
		}
		else
		{
			json_res = handler->second->execute(json_req["data"], player);
		}

#ifdef DEBUG
		if (json_res["result"].is_string())
		{
			const auto result = json_res["result"].get<std::string>();
			console::debug("[Endpoint] Command \"%s\" (%lli) result: %s\n", msgid_str.data(), id, result.data());
		}
		else
		{
			console::warning("[Endpoint] Command \"%s\" has no result set\n", msgid_str.data());
		}
#endif

		return this->encrypt_response(json_req, json_res, player);
	}

	void endpoint_handler::print_handler_name([[ maybe_unused ]] const std::string& name)
	{
		console::log("Registering command \"%s\"\n", name.data());
	}
}
