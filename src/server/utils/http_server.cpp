#include <std_include.hpp>

#include "http_server.hpp"

#include <utils/io.hpp>

namespace utils
{
	http_connection::http_connection(mg_connection* c)
		: conn_(c)
	{
	}

	void http_connection::reply(const std::uint32_t code, const std::string& headers, const std::string& data) const
	{
		mg_http_reply(this->conn_, code, headers.data(), "%s", data.data());
	}

	void http_connection::clear_task()
	{
		const auto task = this->get_data<response_t>();
		if (task != nullptr)
		{
			utils::memory::get_allocator()->free(task);
		}
	}

	http_server::http_server()
	{
		this->set_ports(80, 443);
	}

	void http_server::set_request_handler(const event_handler_t& handler)
	{
		this->request_handler.emplace(handler);
	}

	void http_server::event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data)
	{
		const auto inst = reinterpret_cast<http_server*>(fn_data);
		http_connection conn = c;
		const auto response = conn.get_data<response_t>();

		if (ev == MG_EV_HTTP_MSG)
		{
			const auto http_message = static_cast<mg_http_message*>(ev_data);

			if (!inst->request_handler.has_value())
			{
				conn.reply(500);
				return;
			}

			const auto uri = std::string(http_message->uri.ptr, http_message->uri.len);
			const auto body = std::string(http_message->body.ptr, http_message->body.len);

			request_params params{};
			params.body = body;
			params.uri = uri;

			inst->request_handler->operator()(conn, params);
		}
		else if (ev == MG_EV_POLL)
		{
			if (response == nullptr || !response->valid() || response->wait_for(0ms) != std::future_status::ready)
			{
				return;
			}

			try
			{
				const auto res = response->get();
				conn.reply(res.code, res.headers, res.body);
			}
			catch (const std::exception& e)
			{
				printf("error: %s\n", e.what());
			}

			conn.clear_task();
		}
	}

	void http_server::set_ports(const std::uint16_t http_port, const std::uint16_t https_port)
	{
		this->http_port_ = http_port;
		this->https_port_ = https_port;

		this->http_url_ = std::format("http://0.0.0.0:{}", http_port);
		this->https_url_ = std::format("https://0.0.0.0:{}", https_port);
	}

	void http_server::set_tls(const std::string& cert, const std::string& key)
	{
		this->tls_cert_ = utils::io::read_file(cert);
		this->tls_key_ = utils::io::read_file(key);

		this->tls_options_.server_cert = mg_str_n(this->tls_cert_.data(), this->tls_cert_.size());
		this->tls_options_.server_key = mg_str_n(this->tls_key_.data(), this->tls_key_.size());

		this->using_tls_ = true;
	}

	void http_server::start()
	{
		mg_mgr_init(&this->manager_);

		//mg_log_set(3);

		if (this->using_tls_)
		{
			mg_tls_ctx_init(&this->manager_, &this->tls_options_);
			mg_http_listen(&this->manager_, this->https_url_.data(), http_server::event_handler, this);
		}

		mg_http_listen(&this->manager_, this->http_url_.data(), http_server::event_handler, this);

		while (!this->killed_)
		{
			mg_mgr_poll(&this->manager_, 1);
		}

		mg_mgr_free(&this->manager_);
	}

	void http_server::shutdown()
	{
		this->killed_ = true;
	}
}
