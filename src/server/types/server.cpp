#include <std_include.hpp>

#include "server.hpp"

#include "platforms/mgostm/mgostm_handler.hpp"
#include "platforms/tppstm/tppstm_handler.hpp"
#include "platforms/tppstmweb/tppstmweb_handler.hpp"
#include "platforms/api/api_handler.hpp"

#include "utils/encoding.hpp"
#include "utils/http_server.hpp"
#include "utils/config.hpp"

#include <utils/string.hpp>
#include <utils/cryptography.hpp>
#include <utils/io.hpp>

namespace emulator
{
	server::server()
	{
		this->register_handler<mgo::mgostm_handler>("mgostm");
		this->register_handler<tpp::tppstm_handler>("tppstm");
		this->register_handler<tppstmweb_handler>("tppstmweb");
		this->register_handler<api_handler>("api");
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
		headers.append(std::format("Content-Type: {}\r\n", handler->second->get_content_type()));
		headers.append("Keep-Alive: timeout=5, max=100\r\n");
		headers.append("Connection: Keep-Alive\r\n");

		response.headers = headers;
		response.body = result_opt.value();

		return response;
	}

	void server::request_handler(const utils::request_params& request, utils::response_params& response)
	{
		response.code = 200;

		try
		{
			const auto endpoint_params = utils::string::split(request.uri.substr(1), '/');
			if (endpoint_params.size() < 2)
			{
				return;
			}

			const auto& platform = endpoint_params[0];
			const auto& endpoint = endpoint_params[1];

			response = this->handle_request(request, platform, endpoint, request.body);
		}
		catch (const std::exception& e)
		{
			console::error("[server] error handling request: %s\n", e.what());
			response.code = 500;
		}
	}

	bool server::start()
	{
		const auto http_port = config::get<std::uint16_t>("http_port");
		const auto https_port = config::get<std::uint16_t>("https_port");
		const auto cert_file = config::get<std::string>("cert_file");
		const auto key_file = config::get<std::string>("key_file");

		this->http_server.set_ports(http_port, https_port);

		if (!key_file.empty() && !cert_file.empty())
		{
			this->http_server.set_tls(cert_file, key_file);
		}

		this->http_server.set_request_handler([&](const utils::request_params& request, utils::response_params& response)
		{
			this->request_handler(request, response);
		});

		return this->http_server.start();
	}

	void server::run_frame()
	{
		this->http_server.run_frame();
	}
}
