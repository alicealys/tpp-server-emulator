#include <std_include.hpp>

#include "http_server.hpp"

#include "component/console.hpp"

#include <utils/io.hpp>

namespace utils
{
	namespace
	{
		std::size_t thread_index{};
	}

	http_connection::http_connection(mg_connection* c)
		: conn_(c)
	{
	}

	void http_connection::reply(const std::uint32_t code, const std::string& headers, const std::string& data) const
	{
		this->reply(code, headers.data(), data.data());
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
		const auto task = this->get_data<task_data_t>();
		if (task == nullptr)
		{
			return;
		}

		if (task->thread.joinable())
		{
			task->thread.join();
		}

		delete task;
		std::memset(this->conn_->data, 0, MG_DATA_SIZE);
	}

	void http_connection::reply_async(const std::function<response_params()>& cb) const
	{
		const auto index = thread_index++;
		auto task = new task_data_t{};

#ifdef HTTP_DEBUG
		task->start = std::chrono::high_resolution_clock::now();
		console::debug("[HTTP Server] [Request %lli] Started\n", index);
#endif
		task->thread = std::thread([=]()
		{
			const auto result = cb();
			task->params = result;
			task->done = true;
#ifdef HTTP_DEBUG
			const auto now = std::chrono::high_resolution_clock::now();
			console::debug("[HTTP Server] [Request %lli] Finished in %lli msec\n", index,
				std::chrono::duration_cast<std::chrono::milliseconds>(now - task->start).count());
#endif
		});

		this->set_data<task_data_t>(task);
	}

	http_server::http_server()
	{
		this->set_ports(80, 443);
	}

	http_server::~http_server()
	{
		this->shutdown();
	}

	void http_server::set_request_handler(const event_handler_t& handler)
	{
		this->request_handler.emplace(handler);
	}

	void http_server::event_handler(mg_connection* c, const int ev, void* ev_data, void* fn_data)
	{
		const auto inst = reinterpret_cast<http_server*>(fn_data);
		http_connection conn = c;
		const auto task = conn.get_data<task_data_t>();

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
			if (task == nullptr || !task->done)
			{
				return;
			}

			try
			{
				conn.reply(task->params.code, task->params.headers, task->params.body);
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

	bool http_server::start()
	{
		mg_mgr_init(&this->manager_);

#ifdef DEBUG
		mg_log_set(MG_LL_DEBUG);
#else
		mg_log_set(MG_LL_NONE);
#endif

		if (this->using_tls_)
		{
			mg_tls_ctx_init(&this->manager_, &this->tls_options_);
			mg_http_listen(&this->manager_, this->https_url_.data(), http_server::event_handler, this);
		}

		const auto conn = mg_http_listen(&this->manager_, this->http_url_.data(), http_server::event_handler, this);
		return conn != nullptr;
	}

	void http_server::run_frame()
	{
		mg_mgr_poll(&this->manager_, 1);
	}

	void http_server::shutdown()
	{
		mg_mgr_free(&this->manager_);
	}
}
