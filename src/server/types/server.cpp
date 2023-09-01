#include <std_include.hpp>

#include "server.hpp"

#include "platforms/steam/steam_handler.hpp"

#include "utils/encoding.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>

namespace tpp
{
	namespace
	{
		std::string mg_str_to_string(mg_str& str)
		{
			if (str.ptr == nullptr)
			{
				return {};
			}

			return {str.ptr, str.len};
		}
	}

	server::server()
	{
		this->register_handler<steam_handler>("tppstm");
	}

	void server::set_connection(mg_connection* connection)
	{
		this->connection_ = connection;
	}

	mg_connection* server::get_connection()
	{
		return this->connection_;
	}

	void server::handle_request(mg_connection* connection, const std::string& platform, const std::string& endpoint, const std::string& data)
	{
		const auto handler = this->handlers_.find(platform);
		if (handler == this->handlers_.end())
		{
			return;
		}

		const auto result_opt = handler->second->handle_endpoint(endpoint, data);
		if (!result_opt.has_value())
		{
			return;
		}

		const auto encoded_response = this->encode_response(result_opt.value());

		std::string headers;
		headers.append("Set-Cookie: \r\n");
		headers.append("Content-Type: text/plain\r\n");
		headers.append("Keep-Alive: timeout=5, max=100\r\n");
		headers.append("Connection: Keep-Alive\r\n");

		mg_http_reply(connection, 200, headers.data(), "%s", encoded_response.data());
	}

	std::string server::encode_response(const std::string& data)
	{
		return utils::encoding::split_into_lines(data);
	}
	
	std::optional<std::string> server::decode_request(const std::string& data)
	{
		if (!data.starts_with("httpMsg="))
		{
			return {};
		}

		auto result = data.substr(8);
		return utils::encoding::decode_url_string(result);
	}

	void server::event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data)
	{
		if (ev != MG_EV_HTTP_MSG)
		{
			return;
		}

		const auto inst = reinterpret_cast<server*>(fn_data);

		const auto http_message = static_cast<mg_http_message*>(ev_data);
		const auto url = mg_str_to_string(http_message->uri);
		const auto body = mg_str_to_string(http_message->body);

		const auto endpoint_params = utils::string::split(url.substr(1), '/');
		const auto& platform = endpoint_params[0];
		const auto& endpoint = endpoint_params[1];

		const auto request_opt = inst->decode_request(body);
		if (!request_opt.has_value())
		{
			return;
		}

		const auto& request = request_opt.value();
		inst->handle_request(c, platform, endpoint, request);
	}

	void server::start()
	{
		mg_mgr mgr{};

		mg_mgr_init(&mgr);

		printf("starting server\n");

		const auto conn = mg_http_listen(&mgr, ":80", server::event_handler, this);
		if (conn == nullptr)
		{
			return;
		}

		this->set_connection(conn);

		printf("server started\n");

		while (!this->killed_)
		{
			mg_mgr_poll(&mgr, 1000);
		}

		mg_mgr_free(&mgr);
	}
}
