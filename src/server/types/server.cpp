#include <std_include.hpp>

#include "server.hpp"

#include "platforms/tppstm/tppstm_handler.hpp"
#include "platforms/tppstmweb/tppstmweb_handler.hpp"

#include "utils/encoding.hpp"
#include "utils/http_server.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>
#include <utils/io.hpp>

namespace tpp
{
	server::server()
	{
		this->register_handler<tppstm_handler>("tppstm");
		this->register_handler<tppstmweb_handler>("tppstmweb");
	}

	utils::response_params server::handle_request(const utils::request_params& params, const std::string& platform, 
		const std::string& endpoint, const std::string& data)
	{
		utils::response_params response;
		response.code = 200;

		const auto handler = this->handlers_.find(platform);
		if (handler == this->handlers_.end())
		{
			return response;
		}

		const auto result_opt = handler->second->handle_endpoint(params, endpoint, data);
		if (!result_opt.has_value())
		{
			return response;
		}

		std::string headers;
		headers.append("Set-Cookie: \r\n");
		headers.append("Content-Type: text/plain\r\n");
		headers.append("Keep-Alive: timeout=5, max=100\r\n");
		headers.append("Connection: Keep-Alive\r\n");

		response.headers = headers;
		response.body = result_opt.value();

		return response;
	}

	void server::request_handler(const utils::http_connection& conn, const utils::request_params& params)
	{
		conn.reply_async([=, this]
		{
			utils::response_params response;
			response.code = 200;

			const auto endpoint_params = utils::string::split(params.uri.substr(1), '/');
			if (endpoint_params.size() < 2)
			{
				return response;
			}

			const auto& platform = endpoint_params[0];
			const auto& endpoint = endpoint_params[1];

			return this->handle_request(params, platform, endpoint, params.body);
		});
	}

	void server::start()
	{
		this->http_server.set_ports(80, 443);
		this->http_server.set_tls("config/cert.pem", "config/key.pem");
		this->http_server.set_request_handler([&](
			const utils::http_connection& conn, const utils::request_params& params)
		{
			this->request_handler(conn, params);
		});

		this->http_server.start();
	}
}
