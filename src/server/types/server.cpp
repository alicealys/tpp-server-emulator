#include <std_include.hpp>

#include "server.hpp"

#include "platforms/steam/steam_handler.hpp"

#include "utils/encoding.hpp"
#include "utils/http_server.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>
#include <utils/io.hpp>

namespace tpp
{
	server::server()
	{
		this->register_handler<steam_handler>("tppstm");
	}

	utils::response_params server::handle_request(const std::string& platform, const std::string& endpoint, const std::string& data)
	{
		utils::response_params response;
		response.code = 200;

		const auto handler = this->handlers_.find(platform);
		if (handler == this->handlers_.end())
		{
			return response;
		}

		const auto result_opt = handler->second->handle_endpoint(endpoint, data);
		if (!result_opt.has_value())
		{
			return response;
		}

		const auto encoded_response = this->encode_response(result_opt.value());

		std::string headers;
		headers.append("Set-Cookie: \r\n");
		headers.append("Content-Type: text/plain\r\n");
		headers.append("Keep-Alive: timeout=5, max=100\r\n");
		headers.append("Connection: Keep-Alive\r\n");

		response.headers = headers;
		response.body = encoded_response;

		return response;
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

			const auto request_opt = this->decode_request(params.body);
			if (!request_opt.has_value())
			{
				return response;
			}

			const auto& request = request_opt.value();

			return this->handle_request(platform, endpoint, request);
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
