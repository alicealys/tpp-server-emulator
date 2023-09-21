#include <std_include.hpp>

#include "http_server.hpp"

#include <utils/io.hpp>

namespace utils
{
	namespace
	{
		std::size_t thread_index{};
		std::unordered_map<std::size_t, thread_data_t> thread_list;
	}

	http_connection::http_connection(mg_connection* c)
		: conn_(c)
	{
	}

	void http_connection::reply(const std::uint32_t code, const std::string& headers, const std::string& data) const
	{
		mg_http_reply(this->conn_, code, headers.data(), "%s", data.data());
	}

	void http_connection::reply(const std::uint32_t code, const char* headers, const char* data) const
	{
		mg_http_reply(this->conn_, code, headers, "%s", data);
	}

	void http_connection::reply(const std::function<response_params()>& cb) const
	{
		const auto result = cb();
		this->reply(result.code, result.headers, result.body);
	}

	void http_connection::clear_task()
	{
		const auto task = this->get_data<thread_data_t>();
		if (task == nullptr)
		{
			return;
		}

		if (task->thread.joinable())
		{
			task->thread.join();
		}

		thread_list.erase(task->index);
		std::memset(this->conn_->data, 0, MG_DATA_SIZE);
	}

	void http_connection::reply_async(const std::function<response_params()>& cb) const
	{
		const auto index = thread_index++;
		auto thread_data = &thread_list[index];

		thread_data->index = index;
		thread_data->thread = std::thread([=]()
		{
			const auto result = cb();
			thread_data->params = result;
			thread_data->done = true;
		});

		this->set_data<thread_data_t>(thread_data);
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
		const auto response = conn.get_data<thread_data_t>();

		switch (ev)
		{
		case MG_EV_HTTP_MSG:
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
			break;
		}
		case MG_EV_POLL:
		{
			if (response == nullptr || !response->done)
			{
				return;
			}

			try
			{
				conn.reply(response->params.code, response->params.headers, response->params.body);
			}
			catch (const std::exception& e)
			{
				printf("error: %s\n", e.what());
			}

			conn.clear_task();
			break;
		}
		case MG_EV_CLOSE:
		{
			conn.clear_task();
			break;
		}
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
